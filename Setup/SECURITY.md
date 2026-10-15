# Security

## Never commit credentials
Wi-Fi passwords and Adafruit IO keys belong only in `secrets.h`, which is git-ignored.
Before opening an issue or PR, remove them from logs, screenshots and code snippets.

## If a key leaks
1. Regenerate your Adafruit IO key (My Key page) and change your Wi-Fi password if it was exposed.
2. Rewriting git history does not un-leak a secret that was public - always rotate it.

## Local web page
The ESP32 dashboard is plain HTTP without authentication. Run it on a trusted network and do not port-forward it to the internet.

## Reporting a vulnerability
Please open a private security advisory on GitHub (Security tab -> Report a vulnerability) rather than a public issue.
