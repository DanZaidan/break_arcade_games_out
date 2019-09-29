char* pak_file;

inline char*
load_pak_file() {
    pak_file = os_get_pak_data().data;
    
    assert(*pak_file == 'G');
    assert(*(pak_file+1) == 'A');
    assert(*(s16*)(pak_file+2) == 1); // Version Number
    assert(*(u32*)(pak_file+4) == ASSET_COUNT);
    
    return pak_file;
}


internal String
load_asset(int asset, u8 *asset_format) {
    String result;
    
    u32 file_location     = *(u32*)(pak_file + HEADER_SIZE + asset*sizeof(u32));
    u32 file_end_location = *(u32*)(pak_file + HEADER_SIZE + (asset+1)*sizeof(u32));
    
    if (asset_format) *asset_format = *((u8*)pak_file + file_location);
    file_location++;
    
    result.data = pak_file + file_location;
    result.size = file_end_location - file_location;
    
    return result;
}


//////////
// Sounds


OS_JOB_CALLBACK(async_load_ogg_callback) {
    *(Loaded_Sound*)data3 = load_ogg_from_memory((String){data1, (s64)data2});
}

inline void
async_load_ogg_from_memory(String s, Loaded_Sound *loaded_sound) {
    os_add_job_to_queue(general_queue, async_load_ogg_callback, s.data, (void*)s.size, loaded_sound);
}


Loaded_Sound sounds[LAST_SOUND-FIRST_SOUND+1];


internal void
load_sound_from_pak(int asset, Loaded_Sound *result) {
    
    u8 asset_format;
    String sound = load_asset(asset, &asset_format);
    
    if (asset_format == ASSET_FORMAT_WAV) {
        *result = load_wav_from_memory(sound);
    } else if (asset_format == ASSET_FORMAT_OGG) {
        async_load_ogg_from_memory(sound, result);
    } else {
        invalid_code_path;
    }
}


/////////
// Bitmap

struct {
    u32 *pixels;
    int width, height;
} typedef Bitmap;

Bitmap bitmaps[LAST_BITMAP+1];

internal Bitmap
load_png(char *path) {
    Bitmap result;
    
    String image = os_read_entire_file(path);
    int n;
    stbi_set_flip_vertically_on_load(1);
    result.pixels = (u32*)stbi_load_from_memory((void*)image.data, (int)image.size, &result.width, &result.height, &n, 4);
    u32 *pixel = result.pixels;
    for (int y = 0; y < result.height; y++) {
        for (int x = 0; x < result.width; x++) {
            u8 r = (u8)(*pixel & 0x0000ff);
            u8 g = (u8)((*pixel & 0x00ff00) >> 8);
            u8 b = (u8)((*pixel & 0xff0000) >> 16);
            u8 a = (u8)((*pixel & 0xff000000) >> 24);
            *pixel++ =  b | (g << 8) | (r << 16) | (a << 24);
        }
    }
    
    os_free_file(image);
    return result;
}


internal Bitmap
load_png_from_pak(int asset) {
    Bitmap result;
    
    u8 format;
    String image = load_asset(asset, &format);
    assert(format == ASSET_FORMAT_PNG);
    
    int n;
    stbi_set_flip_vertically_on_load(1);
    result.pixels = (u32*)stbi_load_from_memory((void*)image.data, (int)image.size, &result.width, &result.height, &n, 4);
    u32 *pixel = result.pixels;
    for (int y = 0; y < result.height; y++) {
        for (int x = 0; x < result.width; x++) {
            u8 r = (u8)(*pixel & 0x0000ff);
            u8 g = (u8)((*pixel & 0x00ff00) >> 8);
            u8 b = (u8)((*pixel & 0xff0000) >> 16);
            u8 a = (u8)((*pixel & 0xff000000) >> 24);
            *pixel++ =  b | (g << 8) | (r << 16) | (a << 24);
        }
    }
    
    return result;
}

inline Bitmap*
get_bitmap(int asset) {
    return bitmaps + asset;
}

inline Loaded_Sound*
get_sound(int asset) {
    return sounds + (asset - FIRST_SOUND);
}