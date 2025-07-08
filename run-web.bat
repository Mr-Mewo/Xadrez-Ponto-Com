@echo off
echo Starting backend server...
start /B cmd /c "cd backend && cargo run"
timeout /t 2 /nobreak >nul
echo Starting frontend development server...
cd frontend
trunk clean
trunk serve
