import WebSocket from "ws";

const DWARF_IP = "192.168.88.1"; // anpassen
const WS_URL = `ws://${DWARF_IP}:9900/ws`; // typischer Port

const delay = ms => new Promise(r => setTimeout(r, ms));

const socket = new WebSocket(WS_URL);

socket.on("open", async () => {
  console.log("Connected to DWARF");

  const send = (params, label) => {
    const msg = {
      cmd: "set_isp_settings",
      params
    };
    console.log("→", label, msg.params);
    socket.send(JSON.stringify(msg));
  };

  // Kamera einschalten (wichtig!)
  socket.send(JSON.stringify({ cmd: "turnOnCamera" }));
  await delay(2000);

  // Reihenfolge bewusst gewählt (von ungefährlich → kritisch)
  send({ brightness: 50 }, "brightness");
  await delay(1500);

  send({ contrast: 40 }, "contrast");
  await delay(1500);

  send({ saturation: 60 }, "saturation");
  await delay(1500);

  send({ sharpness: 20 }, "sharpness");
  await delay(1500);

  send({ gamma: 2.2 }, "gamma");
  await delay(1500);

  send({ denoise: 20 }, "denoise");
  await delay(1500);

  send({ auto_white_balance: false }, "awb off");
  await delay(1000);

  send({
    white_balance_mode: "manual",
    wb_r_gain: 120,
    wb_g_gain: 100,
    wb_b_gain: 140
  }, "manual wb");
  await delay(2000);

  send({ auto_exposure: false }, "ae off");
  await delay(1000);

  send({ exposure: 20000 }, "exposure 20ms");
  await delay(2000);

  send({ gain: 120 }, "gain");
  await delay(2000);

  send({ flip: false, mirror: false }, "orientation");
  await delay(1500);

  send({ hdr: false }, "hdr off");
  await delay(1500);

  console.log("ISP test sequence finished");
});

socket.on("message", msg => {
  console.log("←", msg.toString());
});

socket.on("error", err => {
  console.error("WebSocket error:", err);
});
