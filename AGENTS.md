# Repository Guidelines

## Project Structure & Module Organization
`llfcchat/` is the Qt 6 desktop client. Source, headers, and `.ui` forms live together; assets are in `res/`, `static/`, and `style/`; generated output belongs under `llfcchat/build/` and `llfcchat/bin/`.

`workspace/` contains backend services. `GateServer/`, `StatusServer/`, `ResourceServer/`, `ChatServer/`, and `ChatServer2/` are Visual Studio C++/gRPC services. `VarifyServer/` is the Node.js gRPC verification service. Each C++ service keeps `message.proto`, generated `message.pb.*` / `message.grpc.pb.*`, `config.ini`, and a `start.bat` helper beside the source.

## Build, Test, and Development Commands
- `qmake llfcchat.pro && mingw32-make` in `llfcchat/` from a Qt MinGW shell: build the desktop client.
- `msbuild GateServer.sln /p:Configuration=Debug /p:Platform=x64` in a service directory: build a C++ backend. Swap `GateServer` for the service you changed.
- `./start.bat` in any C++ service directory: regenerate protobuf and gRPC stubs from `message.proto`.
- `npm install` then `npm run serve` in `workspace/VarifyServer/`: install dependencies and start the verification service.

## Coding Style & Naming Conventions
Follow the surrounding file style and avoid reformat-only diffs. For new code, prefer 4-space indentation, PascalCase for C++ classes (`ConfigMgr`, `ChatDialog`), and matching base names across `.h`, `.cpp`, and `.ui` files. Keep JS modules lowercase (`server.js`, `redis.js`) and export small focused helpers. Do not hand-edit generated `message.pb.*` or `message.grpc.pb.*` files.

## Testing Guidelines
No automated test suite is committed. Minimum validation: rebuild every touched component, start the affected service locally, and smoke-test the related flow. Examples: run `VarifyServer` and call `GetVarifyCode`; launch the Qt client and verify login/chat; confirm changed C++ services bind to the ports in `config.ini`. Note any manual coverage gaps in the PR.

## Commit & Pull Request Guidelines
This workspace snapshot does not include `.git` history, so use clear, imperative commit messages, preferably Conventional Commit style, such as `feat: add GateServer retry logging` or `fix: regenerate ResourceServer proto stubs`. PRs should summarize affected modules, list config or proto changes, include manual test steps, and attach screenshots for UI edits.

## Security & Configuration Tips
`config.ini` files and `workspace/VarifyServer/config.json` contain local ports and credentials. Treat them as local-only values, replace secrets with placeholders before sharing, and do not commit runtime artifacts from `build/`, `x64/`, `Debug/`, `.vs/`, or `node_modules/`.
