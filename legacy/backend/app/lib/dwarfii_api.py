"""
DWARF II API - Python Port von dwarfii_api
Basierend auf: https://github.com/stevejcl/dwarfii_api

1:1 Nachbau der JavaScript-Library in Python
"""
import asyncio
import logging
from typing import Optional, Dict, Any, Callable
from collections import deque
import websockets

from ..services.proto import base_pb2
from ..utils.constants import *

logger = logging.getLogger(__name__)


# ============================================================================
# API Codes & Constants (aus api_codes.js)
# ============================================================================

DWARF_IP = "192.168.8.29"

def ws_url(ip: str, proxy_url: Optional[str] = None, use_https: bool = False) -> str:
    """WebSocket URL generieren"""
    if proxy_url:
        protocol = "wss" if use_https else "ws"
        url = f"{protocol}://{proxy_url}/?target=ws://{ip}:9900"
        logger.info(f"Starting Web Socket: {url}")
        return url
    
    url = f"ws://{ip}:9900"
    logger.info(f"Starting Web Socket: {url}")
    return url


# Kamera-Modi
MODE_AUTO = 0
MODE_MANUAL = 1

# Binning
BINNING_1X1 = 0
BINNING_2X2 = 1

# File Types
FILE_FITS = 0
FILE_TIFF = 1

# IR Cut
IR_CUT = 0
IR_PASS = 1


# ============================================================================
# WebSocket Handler (aus websocket_class.js)
# ============================================================================

class WebSocketHandler:
    """
    WebSocket Handler für DWARF II
    1:1 Port der JavaScript WebSocketHandler Klasse
    
    HINWEIS: Singleton Pattern deaktiviert für FastAPI
    """
    
    def __init__(self, ip_dwarf: Optional[str] = None):
        """Initialisierung"""
        
        # Connection
        self.socket = None
        self.is_opened = False
        
        # Configuration
        self.ip_dwarf = ip_dwarf
        self.proxy_url = None
        self.use_https = False
        
        # Packet handling
        self.ws_packet = {}
        
        # Callbacks
        self.is_callback_messages = False
        self.packet_callback_messages = {}
        self.is_callback_errors = False
        self.packet_callback_errors = {}
        self.is_callback_connect_states = False
        self.packet_callback_connect_states = {}
        self.callback_reconnect_function = None
        
        # Timers
        self.close_socket_timer = None
        self.close_timer_handler = lambda: None
        self.on_stop_timer_handler = lambda: None
        
        # State flags
        self.keep_connection = False
        self.is_running = False
        self.is_sending = False
        self.is_receiving = False
        self.is_stopping = False
        self.is_buffered = False
        
        # Queue
        self.sending_queue = deque()
        
        # Ping/Pong
        self.is_pong_received = False
        self.is_ping_stopped = True
        self.signal_ping_stop = False
        self.ping_interval = 10
        
        # Reconnect
        self.nb_reconnect_default = 3
        self.nb_reconnect = 3
        self.nb_ping_error_default = 10
        self.nb_ping_error = 10
        
        # Protocol
        self.major_version = 2
        self.minor_version = 0
        self.device_id = 1  # 1: DWARF II, 2: DWARF3
        self.client_id = ""

        # Close state
        self.last_close_code = None
        self.last_close_reason = None
        self.close_event = asyncio.Event()
        
        logger.info(f"Creating WebSocketHandler with IP: {ip_dwarf}")
    
    async def set_new_ip_dwarf(self, ip_dwarf: str):
        """Set new IP address"""
        logger.debug(f"setIpDwarf: {ip_dwarf}")
        if ip_dwarf != self.ip_dwarf:
            logger.debug(f"New IP received, closing old one: {self.ip_dwarf}")
            await self.close()
            await asyncio.sleep(1.0)
        self.ip_dwarf = ip_dwarf
        logger.debug(f"New IP: {self.ip_dwarf}")
    
    async def set_proxy_url(self, proxy_url: Optional[str] = None):
        """Set proxy URL"""
        if not proxy_url:
            logger.debug("Resetting Proxy URL value")
        else:
            logger.debug(f"Setting Proxy URL: {proxy_url}")
        
        if proxy_url != self.proxy_url:
            logger.debug(f"New Proxy URL received, closing connection: {self.proxy_url}")
            await self.close()
            await asyncio.sleep(1.0)
        
        self.proxy_url = proxy_url
        if self.proxy_url:
            logger.debug(f"Using Proxy URL: {self.proxy_url}")
        else:
            logger.debug("Proxy URL reset")
    
    async def set_https_mode(self, use_https: bool):
        """Set HTTPS mode for proxy"""
        logger.debug(f"Setting HTTPS mode: {use_https}")
        self.use_https = use_https
    
    def is_connected(self) -> bool:
        """Check if connected"""
        return self.is_opened and self.socket is not None
    
    async def open(self):
        """
        Open WebSocket connection
        1:1 Port der open() Funktion
        """
        if self.is_opened:
            logger.warning("Socket already opened")
            return
        
        if not self.ip_dwarf:
            logger.error("No IP address set")
            return
        
        try:
            # Reset state
            self.is_opened = False
            self.socket = None
            
            # Create WebSocket URL
            url = ws_url(self.ip_dwarf, self.proxy_url, self.use_https)
            
            logger.info("Launch open new Socket")
            
            # Connect
            new_socket = await websockets.connect(
                url,
                ping_interval=None,
                ping_timeout=None
            )
            
            # Set socket
            self.socket = new_socket
            self.is_opened = True
            
            logger.debug(f"New socket created: {new_socket}")
            
            if not self.proxy_url:
                logger.debug(f"websocket_class: open... on IP: {self.ip_dwarf}")
            else:
                logger.debug(f"websocket_class: open... on IP: {self.ip_dwarf} using proxy: {self.proxy_url}")
            
            # Start tasks
            self.is_running = True
            asyncio.create_task(self._receive_loop())
            asyncio.create_task(self._send_loop())
            
            # Callback
            if self.is_callback_connect_states:
                for callback in self.packet_callback_connect_states.values():
                    callback(True)
            
        except Exception as e:
            logger.error(f"Error opening socket: {e}")
            self.is_opened = False
            
            if self.is_callback_errors:
                for callback in self.packet_callback_errors.values():
                    callback(str(e))
    
    async def close(self):
        """Close WebSocket connection"""
        logger.info("Closing WebSocket")
        
        self.is_running = False
        self.is_stopping = True
        
        if self.socket:
            await self.socket.close()
            self.socket = None
        
        self.is_opened = False
        self.is_stopping = False
        
        # Callback
        if self.is_callback_connect_states:
            for callback in self.packet_callback_connect_states.values():
                callback(False)
    
    async def _receive_loop(self):
        """
        Receive loop
        1:1 Port der onmessage Handler
        """
        logger.info("📡 Receive loop started")
        
        while self.is_running and self.socket:
            try:
                # Wait while sending or buffered
                while self.is_sending or self.is_buffered:
                    await asyncio.sleep(0.01)
                
                logger.debug("websocket_class: onmessage function starting...")
                
                self.is_receiving = True
                
                # Receive message
                data = await self.socket.recv()
                
                if isinstance(data, bytes):
                    await self._handle_message(data)
                elif isinstance(data, str) and data == "pong":
                    logger.debug("Pong received")
                    self.is_pong_received = True
                
                self.is_receiving = False
                
                logger.debug("websocket_class: onmessage function ending...")
                
            except websockets.exceptions.ConnectionClosed as exc:
                logger.warning(
                    "WebSocket connection closed (code=%s, reason=%s)",
                    getattr(exc, "code", "?"),
                    getattr(exc, "reason", "?")
                )
                await self._handle_connection_closed(
                    getattr(exc, "code", None),
                    getattr(exc, "reason", None)
                )
                break
            except Exception as e:
                logger.error(f"Error in receive loop: {e}")
                break
        
        self.is_running = False
        logger.info("📡 Receive loop ended")

    async def _send_loop(self):
        """
        Send loop
        1:1 Port der send() Funktion
        """
        await asyncio.sleep(0.25)  # Initial delay wie in JS
        
        while not self.is_running:
            await asyncio.sleep(0.01)
        
        logger.debug("websocket_class: send function...")
        
        self.is_sending = False
        
        while not self.is_stopping:
            await asyncio.sleep(0.01)
            
            if (not self.is_buffered and 
                not self.is_sending and 
                len(self.sending_queue) > 0 and 
                self.is_connected()):
                
                logger.debug("websocket_class: send function starting...")
                self.is_sending = True
                
                self.ws_packet = self.sending_queue.popleft()
                
                # Send command
                if self.ws_packet:
                    try:
                        await self.socket.send(self.ws_packet)
                        logger.info(f"websocket_class: sending buffer = {len(self.ws_packet)} bytes")
                        await asyncio.sleep(0.1)  # Delay wie in JS
                    except websockets.exceptions.ConnectionClosed as exc:
                        logger.error(
                            "Send failed - connection closed (code=%s, reason=%s)",
                            getattr(exc, "code", "?"),
                            getattr(exc, "reason", "?")
                        )
                        await self._handle_connection_closed(
                            getattr(exc, "code", None),
                            getattr(exc, "reason", None)
                        )
                        break
                else:
                    logger.error(f"websocket_class: sending buffer empty")
                
                self.is_sending = False

    async def _handle_connection_closed(self, code=None, reason=None):
        """Reset state when socket is closed"""
        self.last_close_code = code
        self.last_close_reason = reason
        self.close_event.set()
        self.is_opened = False
        self.is_running = False
        self.is_sending = False
        self.is_receiving = False
        self.is_stopping = False
        if self.socket:
            try:
                await self.socket.close()
            except Exception:
                pass
        self.socket = None
        # Queue bleiben, aber nicht senden solange keine Verbindung besteht
        logger.debug("Connection marked as closed")

        if self.is_callback_errors:
            for callback in self.packet_callback_errors.values():
                try:
                    callback({"code": code, "reason": reason})
                except Exception as exc:
                    logger.error(f"Error callback failed: {exc}")

    async def _handle_message(self, data: bytes):
        """
        Handle received message
        1:1 Port der handleMessage Funktion
        """
        try:
            # Parse WsPacket
            packet = base_pb2.WsPacket()
            packet.ParseFromString(data)
            
            logger.info(
                f"📦 Packet received: Module={packet.module_id}, "
                f"CMD={packet.cmd}, Type={packet.type}"
            )
            
            # Analyze packet (würde normalerweise analyzePacket aufrufen)
            result_data = {
                "module_id": packet.module_id,
                "cmd": packet.cmd,
                "type": packet.type,
                "data": packet.data
            }
            
            # Parse ComResponse für alle Types (nicht nur Type=1)
            try:
                com_response = base_pb2.ComResponse()
                com_response.ParseFromString(packet.data)
                result_data["code"] = com_response.code
                logger.debug(f"ComResponse parsed: code={com_response.code}")
            except Exception as e:
                logger.debug(f"Could not parse as ComResponse: {e}")
            
            # Log result
            logger.info(f"📨 Result: Module={result_data['module_id']}, CMD={result_data['cmd']}, Type={result_data['type']}, Code={result_data.get('code', 'N/A')}")
            
            # Callback für ALLE Nachrichten (nicht nur Type=1)
            if self.is_callback_messages:
                for callback in self.packet_callback_messages.values():
                    callback("Message received", result_data)
            
        except Exception as e:
            logger.error(f"Error handling message: {e}")
    
    def send_packet(self, packet_data: bytes):
        """
        Add packet to sending queue
        """
        self.sending_queue.append(packet_data)
        logger.debug(f"Packet added to queue. Queue length: {len(self.sending_queue)}")
    
    def create_packet(
        self,
        module_id: int,
        cmd: int,
        data: bytes
    ) -> bytes:
        """
        Create WsPacket
        1:1 Port der createPacket Funktion
        """
        packet = base_pb2.WsPacket()
        packet.major_version = self.major_version
        packet.minor_version = self.minor_version
        packet.device_id = self.device_id
        packet.module_id = module_id
        packet.cmd = cmd
        packet.type = 0  # TYPE_REQUEST
        packet.data = data
        packet.client_id = self.client_id
        
        serialized = packet.SerializeToString()
        
        logger.debug(f"Packet created: Module={module_id}, CMD={cmd}")
        logger.debug(f"Packet details: major_v={self.major_version}, minor_v={self.minor_version}, device_id={self.device_id}")
        logger.debug(f"Packet data length: {len(data)} bytes")
        logger.debug(f"Serialized packet: {serialized.hex()} ({len(serialized)} bytes)")
        
        return serialized
    
    def register_message_callback(self, name: str, callback: Callable):
        """Register callback for messages"""
        self.is_callback_messages = True
        self.packet_callback_messages[name] = callback
    
    def unregister_message_callback(self, name: str):
        """Unregister message callback"""
        if name in self.packet_callback_messages:
            del self.packet_callback_messages[name]
        if len(self.packet_callback_messages) == 0:
            self.is_callback_messages = False
    
    def register_error_callback(self, name: str, callback: Callable):
        """Register callback for errors"""
        self.is_callback_errors = True
        self.packet_callback_errors[name] = callback
    
    def register_connect_callback(self, name: str, callback: Callable):
        """Register callback for connection state"""
        self.is_callback_connect_states = True
        self.packet_callback_connect_states[name] = callback
