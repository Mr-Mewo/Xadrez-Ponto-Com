@echo off
echo Building backend...
cd backend
cargo build --release
cd ..
echo Building frontend...
cd frontend
trunk clean
trunk build --release
pause
