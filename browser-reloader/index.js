import { WebSocketServer } from "ws";
import { spawn } from "child_process";
import { v4 as uuidv4 } from "uuid";

async function main() {
  const buildId = uuidv4();
  console.log("build id: " + buildId);

  const wss = new WebSocketServer({ port: 3001 });
  const connections = [];
  wss.on("connection", function connection(ws) {
    connections.push(ws);

    ws.send(JSON.stringify({ name: "buildId", content: buildId }));

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

  const httpServer = spawn(
    "http-server",
    ["../cmake-build-debug-emscripten/sandbox/dist", "-p 3002", "-c-1"],
    { shell: true }
  );

  httpServer.stdout.on("data", (data) => {
    // console.log(`spawn stdout: ${data}`);
  });

  httpServer.stderr.on("data", (data) => {
    // console.log(`spawn stderr: ${data}`);
  });

  httpServer.on("error", (code) => {
    // console.log(`spawn error: ${code}`);
  });

  httpServer.on("close", (code) => {
    // console.log(`spawn child process closed with code ${code}`);
  });

  httpServer.on("exit", (code) => {
    // console.log(`spawn child process exited with code ${code}`);
  });

  console.log("server started on http://127.0.0.1:3002");
}

main();
