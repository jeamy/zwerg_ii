import WebSocket from "ws";
import protobuf from "protobufjs";

const DWARF_IP = "192.168.8.22";
const WS_URL = `ws://${DWARF_IP}:9900`;

const delay = ms => new Promise(r => setTimeout(r, ms));

// Minimal WsPacket proto definition
const root = protobuf.Root.fromJSON({
  nested: {
    dwarf: {
      nested: {
        WsPacket: {
          fields: {
            major_version: { type: "uint32", id: 1 },
            minor_version: { type: "uint32", id: 2 },
            device_id: { type: "uint32", id: 3 },
            module_id: { type: "uint32", id: 4 },
            cmd: { type: "uint32", id: 5 },
            type: { type: "uint32", id: 6 },
            data: { type: "bytes", id: 7 },
            client_id: { type: "string", id: 8 }
          }
        },
        ReqStartPanoramaByGrid: {
          fields: {
            rows: { type: "uint32", id: 1 },
            cols: { type: "uint32", id: 2 }
          }
        },
        ComResponse: {
          fields: {
            code: { type: "int32", id: 1 }
          }
        }
      }
    }
  }
});

const WsPacket = root.lookupType("dwarf.WsPacket");
const ReqStartPanoramaByGrid = root.lookupType("dwarf.ReqStartPanoramaByGrid");
const ComResponse = root.lookupType("dwarf.ComResponse");

const ws = new WebSocket(WS_URL);

function sendProtobufCommand(moduleId, cmd, data = Buffer.alloc(0)) {
  const packet = WsPacket.create({
    major_version: 1,
    minor_version: 1,
    device_id: 1,
    module_id: moduleId,
    cmd: cmd,
    type: 0, // Request
    data: data,
    client_id: "test-client-" + Date.now()
  });
  
  const buffer = WsPacket.encode(packet).finish();
  console.log(`→ Protobuf: Module ${moduleId}, Cmd ${cmd}, Size ${buffer.length}, Data hex: ${data.toString('hex')}`);
  ws.send(buffer);
}

ws.on("open", async () => {
  console.log("Connected via Protobuf");
  
  // 1) Open Wide camera (Module 2, Cmd 12000)
  console.log("\n=== Opening Wide Camera ===");
  sendProtobufCommand(2, 12000, Buffer.alloc(0));
  await delay(3000);
  
  // 2) Start Panorama 6x5 (Module 10, Cmd 15500)
  console.log("\n=== Starting Panorama 6x5 ===");
  const panoReq = ReqStartPanoramaByGrid.create({ rows: 6, cols: 5 });
  const panoData = Buffer.from(ReqStartPanoramaByGrid.encode(panoReq).finish());
  sendProtobufCommand(10, 15500, panoData);
  
  // 3) Wait and observe
  await delay(60000);
  
  console.log("\n=== Stopping Panorama ===");
  sendProtobufCommand(10, 15501, Buffer.alloc(0));
  
  await delay(3000);
  ws.close();
});

ws.on("message", (data) => {
  try {
    const packet = WsPacket.decode(new Uint8Array(data));
    console.log(`← Protobuf: Module ${packet.module_id}, Cmd ${packet.cmd}, Type ${packet.type}, Data size ${packet.data.length}`);
    
    // Parse panorama progress (Module 9, Cmd 15219)
    if (packet.module_id === 9 && packet.cmd === 15219 && packet.data.length > 0) {
      // Simple manual parse: field 1 = total, field 2 = completed
      const buf = Buffer.from(packet.data);
      console.log(`   → Progress notification, data hex: ${buf.toString('hex')}`);
    }
    
    // Parse ComResponse for errors
    if (packet.data.length > 0 && packet.type === 1) {
      try {
        const response = ComResponse.decode(packet.data);
        console.log(`   → Response code: ${response.code}`);
      } catch (e) {
        // Not a ComResponse, ignore
      }
    }
  } catch (e) {
    console.error("Failed to parse packet:", e.message);
  }
});

ws.on("error", (err) => {
  console.error("WebSocket error:", err);
});

ws.on("close", () => {
  console.log("Connection closed");
});
