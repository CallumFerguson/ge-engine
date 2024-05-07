import { WebSocketServer } from "ws";
import { spawn } from "child_process";
import * as path from "path";
import { hashElement } from "folder-hash";

async function main() {
  const buildHash = await hashElement("../cmake-build-debug-emscripten/dist");
  console.log("build hash: " + buildHash.hash);

  const wss = new WebSocketServer({ port: 3001 });
  const connections = [];
  wss.on("connection", function connection(ws) {
    connections.push(ws);

    ws.send(JSON.stringify({ name: "buildHash", content: buildHash.hash }));

    ws.on("error", () => {
      const i = connections.indexOf(ws);
      if (i !== -1) {
        connections.splice(i, 1);
      }
    });

    ws.on("close", () => {
      const i = connections.indexOf(ws);
      if (i !== -1) {
        connections.splice(i, 1);
      }
    });
  });

  const dir = spawn(
    "http-server",
    ["../cmake-build-debug-emscripten/dist", "-p 3002", "-c-1"],
    { shell: true }
  );

  dir.stdout.on("data", (data) => {
    // console.log(`spawn stdout: ${data}`);
  });

  dir.stderr.on("data", (data) => {
    // console.log(`spawn stderr: ${data}`);
  });

  dir.on("error", (code) => {
    // console.log(`spawn error: ${code}`);
  });

  dir.on("close", (code) => {
    // console.log(`spawn child process closed with code ${code}`);
  });

  dir.on("exit", (code) => {
    // console.log(`spawn child process exited with code ${code}`);
  });

  console.log("server started on http://127.0.0.1:3002");
}

main();
