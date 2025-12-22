import WebSocket from "ws";

const DWARF_IP = "192.168.8.22";
const WS_URL = `ws://${DWARF_IP}:9900/ws`;

const ROWS = 6;
const COLS = 5;
const TOTAL_TILES = ROWS * COLS;

const POLL_INTERVAL_MS = 500;
const CAMERA_WARMUP_MS = 3000;
const POST_STOP_DELAY_MS = 4000;

const delay = ms => new Promise(r => setTimeout(r, ms));

let panoRunning = false;
let lastTileIndex = -1;

const ws = new WebSocket(WS_URL);

ws.on("open", async () => {
  console.log("Connected");

  const send = (cmd, params = null) => {
    const msg = params ? { cmd, params } : { cmd };
    console.log("→", msg);
    ws.send(JSON.stringify(msg));
  };

  /* ---------------------------------------
     1) Kamera einschalten
  --------------------------------------- */
  send("turnOnCamera");
  await delay(CAMERA_WARMUP_MS);

  /* ---------------------------------------
     2) Panorama starten
  --------------------------------------- */
  send("startPano", {
    rows: ROWS,
    cols: COLS,
    overlap: 0.25,
    mode: "wide"
  });

  panoRunning = true;

  /* ---------------------------------------
     3) Status-Polling
  --------------------------------------- */
  while (panoRunning) {
    // harmloser Status-Request
    send("cameraWorkingState");
    await delay(POLL_INTERVAL_MS);
  }

  /* ---------------------------------------
     4) Panorama sauber stoppen
  --------------------------------------- */
  console.log("Panorama finished → sending stopPano");
  send("stopPano");

  await delay(POST_STOP_DELAY_MS);
  ws.close();
});

ws.on("message", msg => {
  try {
    const data = JSON.parse(msg.toString());
    console.log("←", data);

    /* ---------------------------------------
       Firmware-agnostische Auswertung
    --------------------------------------- */

    // Variante A: expliziter Panorama-State
    if (data.pano_state) {
      if (data.pano_state === "finished" || data.pano_state === "idle") {
        panoRunning = false;
      }
    }

    // Variante B: Kamera-Zustand
    if (data.mode && data.state) {
      if (data.mode !== "pano" && data.state === "idle") {
        panoRunning = false;
      }
    }

    // Variante C: Tile-Zähler
    if (typeof data.tile_index === "number") {
      lastTileIndex = data.tile_index;
      if (lastTileIndex + 1 >= TOTAL_TILES) {
        panoRunning = false;
      }
    }

    // Variante D: allgemeines Busy-Flag
    if (data.busy === false && panoRunning) {
      panoRunning = false;
    }

  } catch {
    // ignorieren: nicht jede Message ist JSON
  }
});

ws.on("error", err => {
  console.error("WebSocket error:", err);
});
