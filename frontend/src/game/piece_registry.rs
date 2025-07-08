use std::cell::RefCell;
use std::collections::HashMap;

use crate::game::piece::PieceProps;

// Thread-local registry since Yew runs in single-threaded WebAssembly
thread_local! {
    static PIECE_REGISTRY: RefCell<HashMap<String, PieceProps>>
                            = RefCell::new(HashMap::new());
}

pub fn register_piece(props: &PieceProps) {
    PIECE_REGISTRY.with(|registry| {
        registry
            .borrow_mut()
            .insert(props.sqr_id.clone(), props.clone());
    });
}

pub fn get_piece_handles(sqr_id: &str) -> Option<PieceProps> {
    PIECE_REGISTRY.with(|registry| {
        registry
            .borrow()
            .get(sqr_id)
            .cloned()
    })
}
pub fn update_piece_key(old_key: &str, new_key: &str) {
    PIECE_REGISTRY.with(|registry| {
        let mut registry_borrow = registry.borrow_mut();
        if let Some(mut piece_props) = registry_borrow.remove(old_key) {
            piece_props.sqr_id = new_key.to_string();
            registry_borrow
                .insert(new_key.to_string(), piece_props);
        }
    })
}
