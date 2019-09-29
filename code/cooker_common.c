
/*

File Spec:

16bytes - MAGIC WORD       "GA"
16bytes - VERSION NUMBER   1
32bytes - ASSET COUNT

for every asset:
-32bytes - ASSET LOCATION IN FILE

32bytes - TOTAL SIZE

for every asset:
 -8bytes ASSET_FORMAT
-asset


*/


#define HEADER_SIZE 8

enum {
    ASSET_FORMAT_PNG,
    ASSET_FORMAT_WAV,
    ASSET_FORMAT_OGG,
};

enum {
    // Bitmaps
    B_INVINCIBILITY,
    B_TRIPLE,
    B_COMET,
    B_INV,
    B_TNT,
    B_TURTLE,
    B_STRONG,
    
    B_LOGO_LIGHT,
    B_LOGO_DARK,
    
    LAST_BITMAP = B_LOGO_DARK,
    
    
    // Sounds
    FIRST_SOUND,
    
    S_MENU_MUSIC = FIRST_SOUND,
    S_MAIN_MUSIC,
    
    S_HIT_1,
    S_HIT_2,
    S_HIT_3,
    S_HIT_4,
    S_HIT_5,
    S_HIT_6,
    S_HIT_7,
    S_HIT_8,
    S_HIT_9,
    S_HIT_10,
    S_HIT_11,
    S_HIT_12,
    S_HIT_13,
    S_HIT_14,
    S_HIT_15,
    S_HIT_16,
    
    S_GAME_OVER,
    
    S_FORCEFIELD,
    S_FIREWORKS,
    S_SPRING,
    S_START_GAME,
    S_LOSE_LIFE,
    S_REDIRECT,
    S_BALL,
    S_COMET_BEGIN,
    S_COMET_LOOP,
    S_OLD_SOUND,
    S_POWERUP,
    S_POWERDOWN,
    S_INTERFACE,
    S_PLAYER_WALL,
    S_LOAD_GAME,
    
    S_BRICK_1,
    S_BRICK_2,
    S_BRICK_3,
    S_BRICK_4,
    S_BRICK_5,
    
    S_SINE,
    
    LAST_SOUND = S_SINE,
    
    ASSET_COUNT
};


#if 0

u32 assert_start_location = pak + header_size + sizeof(u32)*S_FORCE_FIELD

asset = pak + assert_start_location;

#endif