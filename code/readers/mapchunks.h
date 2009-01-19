#ifndef MAPCHUNKS_H
#define MAPCHUNKS_H

 enum eMapChunks//ya
    {
        CHUNK_MAP_END_OF_BLOCK      = 0x00,//fin de bloque 
        CHUNK_MAP_CHIPSET           = 0x01,// nombre de la imagen del chipset
        CHUNK_MAP_WIDTH             = 0x02, // altura
        CHUNK_MAP_HEIGHT            = 0x03, // anchura
        CHUNK_MAP_TYPE_OF_LOOP      = 0x0B, // tipo de ciclo del panorama
        CHUNK_MAP_PARALLAX_BACK     = 0x1F, // panorama parrarelo
        CHUNK_MAP_BACKGROUND        = 0x20, // panaroma en pantalla
        CHUNK_MAP_HORIZONTAL_PAN    = 0x21, // banderas de  movimentos 
        CHUNK_MAP_VERTICAL_PAN      = 0x22,  
        CHUNK_MAP_HORIZONTAL_PAN_A  = 0x23, // velocidades del movimiento
        CHUNK_MAP_HORIZONTAL_PAN_SP = 0x24,       
        CHUNK_MAP_VERTICAL_PAN_A    = 0x25, 
        CHUNK_MAP_VERTICAL_PAN_SP   = 0x26,        
        CHUNK_MAP_LOWER_LAYER       = 0x47, // capa de chipset inferior
        CHUNK_MAP_UPPER_LAYER       = 0x48, // capa de chipset superior
        CHUNK_MAP_EVENTS_LAYER      = 0x51, // eventos del mapa        
        CHUNK_MAP_TIMES_SAVED       = 0x5B,  // veces salvado
        CHUNK_MAP_USE_GENERATOR     = 0x28,
        CHUNK_MAP_GEN_MODE          = 0x29,
        CHUNK_MAP_GEN_NUM_TITLES    = 0x30,
        CHUNK_MAP_GEN_WIDTH         = 0x31,
        CHUNK_MAP_GEN_HEIGTH        = 0x32,
        CHUNK_MAP_GEN_SURROUND_MAP  = 0x33,
        CHUNK_MAP_GEN_UPER_WALL     = 0x34,
        CHUNK_MAP_GEN_FLOOR_B       = 0x35,
        CHUNK_MAP_GEN_FLOOR_C       = 0x36,
        CHUNK_MAP_GEN_EXTRA_B       = 0x37,
        CHUNK_MAP_GEN_EXTRA_C       = 0x38,
        CHUNK_MAP_GEN_X_POS         = 0x3C, 
        CHUNK_MAP_GEN_Y_POS         = 0x3D,
        CHUNK_MAP_GEN_CHIPSETS_IDS  = 0x3E
    };
    
    enum eMapEventChunks//ya
    {
        CHUNK_EVENT_NAME            = 0x01, // nombre del evento
        CHUNK_EVENT_X               = 0x02, // posicion en x
        CHUNK_EVENT_Y               = 0x03, // posiscion en y
        CHUNK_EVENT_PAGES           = 0x05 // paginas de comandos
    };

    enum eMapPagedataChunks//ya
    {
		CHUNK_Page_conditions= 0x02,
		CHUNK_Charset_to_use= 0x15,
		CHUNK_Graphic_to_use= 0x16,
		CHUNK_Facing_direction= 0x17,
		CHUNK_Animation_frame= 0x18,
		CHUNK_Transparency= 0x19,
		CHUNK_Movement_type= 0x1F,
		CHUNK_Movement_frequency= 0x20,
		CHUNK_Activation_condition= 0x21,
		CHUNK_Event_height= 0x22,
		CHUNK_Prevent_event_overlap= 0x23,
		CHUNK_Animation_type= 0x24,	
		CHUNK_Movement_speed= 0x25,
		CHUNK_Movement_block= 0x29,
		CHUNK_Script_header= 0x33,
		CHUNK_Script_code= 0x34
   };
   enum eMapPageConditionsChunks
    { 
		CHUNK_Conditions =0x01,		
		CHUNK_Switch_A_ID=0x02,	
		CHUNK_Switch_B_ID=0x03,	
		CHUNK_Variable_ID=0x04,	
		CHUNK_Variable_value=0x05,	
		CHUNK_Item=0x06,	
		CHUNK_Hero=0x07,	
		CHUNK_Clock=0x08
   };
    enum eMapPageMovesChunks
    {   
   		CHUNK_Movement_length=0x0B,
		CHUNK_Movement_commands=0x0C,
		CHUNK_Repeat_movement=0x15,
		CHUNK_Ignore_impossible=0x16
   };
#endif
