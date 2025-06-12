#[tauri::command]
fn close(window: tauri::Window) {
    window.close().unwrap();
}

#[tauri::command]
fn fullscreen(window: tauri::Window) {
    window.set_fullscreen( !window.is_fullscreen().unwrap() ).unwrap();
}

#[tauri::command]
fn maximize(window: tauri::Window) {
    if window.is_maximized().unwrap() {
        window.unmaximize().unwrap();
    }else { 
        window.maximize().unwrap()
    }
}

#[tauri::command]
fn minimize(window: tauri::Window) {
    window.minimize().unwrap();
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
  tauri::Builder::default()
      //
    .invoke_handler(tauri::generate_handler![close, fullscreen, maximize, minimize])
      //
    .setup(|app| {
        if cfg!(debug_assertions) {
            app.handle().plugin(
              tauri_plugin_log::Builder::default()
                .level(log::LevelFilter::Info)
                .build(),
            )?;
        }
        Ok(())
    })
    .run(tauri::generate_context!())
    .expect("error while running tauri application");
}
