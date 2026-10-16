@echo off
title Subir Emplay Mod Menu a GitHub Automaticamente
echo ========================================================
echo   SUBIENDO EMPLAY MOD MENU A TU GITHUB AUTOMATICAMENTE
echo ========================================================
cd /d "%~dp0"

git --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [!] No tienes Git instalado en Windows. Sube los archivos arrastrandolos a GitHub.
    pause
    exit /b
)

git init
git branch -M main
git remote remove origin >nul 2>&1
git remote add origin https://github.com/emanuelskibnididi2341425/Geode-Mod-menu-hecho-por-emanuel.git
git pull origin main --allow-unrelated-histories
git add .
git commit -m "Todo el proyecto arreglado y listo para Android y PC"
git push -u origin main --force

echo.
echo ========================================================
echo   LISTO! YA SE HA SUBIDO TODO A TU GITHUB!
echo   Ve a la pestana Actions o Releases de tu GitHub.
echo ========================================================
pause
