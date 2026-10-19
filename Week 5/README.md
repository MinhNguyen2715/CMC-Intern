# Windows Service

## Install Windows Service
1. Run `cmd` (Admin)
2. Run: `"C:\Path_To_Project\x64\Release\ChatAppService.exe" -install`
3. Expected output: `Windows Service 'ChatAppService' installed SUCCESSFUL!`

## Start and Verify Service Status
1. On `cmd` (Admin): `sc start ChatAppService`
2. Check if port 9999 is actively listening: `sc query ChatAppService`
3. Expected output: Expected Output: The `STATE` field displays `4  RUNNING`

4. Alternate: `services.msc`

## Test Client Silent Autorun Registry Entry
1. Launch `ChatClient.exe`
2. Open the Registry Editor
3. Navigate to `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run`
4. Expected output: A registry value named `MFC_ChatClient` exists and points directly to the executable path of `ChatClient.exe`

## Test Multi-Client Echo Communication & ID Tagging
1. Launch >2 `ChatClient.exe` app
2. Send message on each app
3. Expected result: each app receives its own message in the form: `[Service echo to client #<ID>]: [HH:MM:SS] <message>`

## Testing Service Stop and Uninstallation
1. On `cmd` (Admin): `sc stop ChatAppService`
2. Expected output: `STATE: 3 STOP_PENDING` followed by `STATE: 1 STOPPED` without throwing error

3. Run: `"C:\Path_To_Your_Project\x64\Release\ChatAppService.exe" -uninstall`
4. Expected output: `Windows Service 'ChatAppService' uninstalled SUCCESSFUL!`
