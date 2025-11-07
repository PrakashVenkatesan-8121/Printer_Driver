@echo off

set CERTIFICATE="%~dp0UMDF2Driver1.cer"

certutil -addstore -f root %CERTIFICATE%
certutil -addstore -f TrustedPublisher %CERTIFICATE%
pause
