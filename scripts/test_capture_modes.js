import WebSocket from "ws";

const DWARF_IP = "192.168.88.1"; // anpassen
const WS_URL = `ws://${DWARF_IP}:9900/ws`;

const delay = ms => new Promise(r => setTimeout(r, ms));
const ws = new WebSocket(WS_URL);

ws.on("open", async () => {
  console.log("Connected");

  const send = (cmd, params = {}) => {
    const msg = { cmd, params };
    console.log("→", msg);
    ws.send(JSON.stringify(msg));
  };

  // Kamera an
  send("turnOnCamera");
  await delay(3000);

  /* --- Panorama --- */
  send("startPano", {
    rows: 2,
    cols: 3,
    overlap: 0.2,
    mode: "wide"
  });
  await delay(20000);
  send("stopPano");
  await delay(5000);

  /* --- Timelapse --- */
  send("startTimeLapse", {
    interval_sec: 5,
    duration_sec: 30,
    resolution: "1080p",
    mode: "wide"
  });
  await delay(35000);

  /* --- Video --- */
  send("startVideo", {
    resolution: "1080p",
    fps: 30,
    codec: "h264",
    mode: "wide"
  });
  await delay(10000);
  send("stopVideo");
  await delay(5000);

  /* --- Einzelbild --- */
  send("takePhoto", {
    mode: "tele",
    exposure_us: 20000,
    gain: 100,
    format: "jpg"
  });

  await delay(3000);
  console.log("All capture tests completed");
});

ws.on("message", msg => {
  console.log("←", msg.toString());
});

ws.on("error", err => {
  console.error("WS error", err);
});
