@echo off
echo Docker imaji oluşturuluyor/güncelleniyor...
docker build -t mangus-mud .

echo.
echo Mangus MUD sunucusu başlatılıyor...
FOR /F "tokens=* USEBACKQ" %%F IN (`docker run -d -p 4000:4000 mangus-mud ./mangus`) DO (
SET container_id=%%F
)

echo.
echo ==============================================
echo Mangus MUD sunucusu çalısıyor!
echo.
echo Oyuna bağlanmak için:
echo IP: 127.0.0.1 (localhost)
echo Port: 4000
echo ==============================================
echo.
echo Sunucuyu durdurmak için Ctrl+C'ye basın
echo.

docker logs -f %container_id%
