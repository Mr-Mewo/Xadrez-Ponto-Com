#[macro_export]
macro_rules! log {
    ($($arg:tt)*) => {
        web_sys::console::log_1(&format!($($arg)*).into())
    };
}