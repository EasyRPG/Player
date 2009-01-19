
// =========================================================================
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "../tools/tools.h"#include "stevent.h"
#include "map.h"
#include <iostream>// =========================================================================




stPageMovesEventMap map_reader::PageMovesChunk(FILE * Stream)//movimientos de la pagina
{
	 stPageMovesEventMap moves;
         moves.clear();
         unsigned char Void;
         tChunk ChunkInfo; // informacion del pedazo leido
         int data;
         string name;
         Move_comand_Sound_effect Sound_effect;
         Move_comand_New_Graphic New_Graphic;
         Move_comand_Switch comand_Switch;
         Move_comand My_Move_comand;
         ChunkInfo.ID     = ReadCompressedInteger(Stream); //id 1 de array
         ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
         while(ChunkInfo.ID!=0)
              {
                         switch(ChunkInfo.ID)// tipo de la primera dimencion
                         {
                          case CHUNK_Movement_length:
                               moves.Movement_length= ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_Movement_commands:
                            do
                            {

                             if(ChunkInfo.Length)//si no es 00
                            { ChunkInfo.Length--;
                              fread(&Void, sizeof(char), 1, Stream);}
                              else
                              Void=0;
                             switch(Void)// tipo de la primera dimencion
                             {
                               case 0x20:
                               comand_Switch.Comand=Void;
                               comand_Switch.Switch_ID= ReadCompressedIntegerCount(Stream);
                               ChunkInfo.Length-=CountRead(-1);
                               moves.vcMovement_commands.push_back(comand_Switch);
                               break;
                               case 0x21:
                                 comand_Switch.Comand=Void;
                                 comand_Switch.Switch_ID= ReadCompressedIntegerCount(Stream);
                                 ChunkInfo.Length-=CountRead(-1);
                                 moves.vcMovement_commands.push_back(comand_Switch);
                               break;
                               case 0x22:
                                 New_Graphic.Comand=Void;
                                 data= ReadCompressedIntegerCount(Stream);
                                 ChunkInfo.Length-=CountRead(-1);
                                 ChunkInfo.Length-=data;
                                 New_Graphic.Name_of_graphic=ReadString(Stream, data);
                                 moves.vcMovement_commands.push_back(New_Graphic);
                               break;
                               case 0x23:
                                Sound_effect.Comand=Void;
                                data= ReadCompressedIntegerCount(Stream);
                                ChunkInfo.Length-=CountRead(-1);
                                ChunkInfo.Length-=data;
                                Sound_effect.Name_of_Sound_effect=ReadString(Stream, data);
                                Sound_effect.Volume= ReadCompressedIntegerCount(Stream);
                                ChunkInfo.Length-=CountRead(-1);
                                Sound_effect.Tempo= ReadCompressedIntegerCount(Stream);
                                ChunkInfo.Length-=CountRead(-1);
                                Sound_effect.Balance= ReadCompressedIntegerCount(Stream);
                                ChunkInfo.Length-=CountRead(-1);
                                moves.vcMovement_commands.push_back(New_Graphic);
                               break;
                                case CHUNK_MAP_END_OF_BLOCK:
                               break;
                               default:
                                My_Move_comand.Comand=Void;
                                moves.vcMovement_commands.push_back(My_Move_comand);
                               break;
                              }
                             }while(ChunkInfo.Length);


                               break;
                          case CHUNK_Repeat_movement:
                               moves.Repeat_movement = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_Ignore_impossible:
                               moves.Ignore_impossible = ReadCompressedInteger(Stream);
                               break;

                          case CHUNK_MAP_END_OF_BLOCK:
                               break;
                          default:
                               // saltate un pedazo del tamaño de la longitud
                               while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
                               break;
                          }
                          ChunkInfo.ID     = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
                          if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
                          ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
              }
              return(moves);
}

stPageConditionEventMap map_reader::conditionChunk(FILE * Stream)//una dimencion
{        stPageConditionEventMap Conditions;
         Conditions.clear();
         unsigned char Void;
         tChunk ChunkInfo; // informacion del pedazo leido
         ChunkInfo.ID     = ReadCompressedInteger(Stream); //id 1 de array
         ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
         while(ChunkInfo.ID!=0)
         {
                         switch(ChunkInfo.ID)// tipo de la primera dimencion
                         {
                          case CHUNK_Conditions:
                               Conditions.Conditions = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_Switch_A_ID:
                               Conditions.Switch_A_ID  = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_Switch_B_ID:
                               Conditions.Switch_B_ID  = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_Variable_ID:
                               Conditions.Variable_ID  = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_Variable_value:
                               Conditions.Variable_value  = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_Item:
                               Conditions.Item = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_Hero:
                               Conditions.Hero  = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_Clock:
                               Conditions.Clock  = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_MAP_END_OF_BLOCK:
                               break;
                          default:
                               while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
                               break;
                          }
                          ChunkInfo.ID     = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
                          if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
                          ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
         }
    return (Conditions);
}
vector <stPageEventMap> map_reader::pageChunk(FILE * Stream)
{        std:: vector <stPageEventMap> vcPage;
         stPageEventMap Page;
         stEvent Event_parser;
         Page.clear();
         int id,datatoread=0,datareaded=0;
         datatoread=ReadCompressedInteger(Stream);//numero de datos
         while(datatoread>datareaded) // si no hay mas en el array
         {
         id= ReadCompressedInteger(Stream);//lectura de id 1 de array
             do
              {          ChunkInfo.ID     = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
                         if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
                          ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
                         switch(ChunkInfo.ID)// tipo de la primera dimencion
                         {
                         case CHUNK_Page_conditions:
                              Page.Page_conditions= conditionChunk(Stream);
                              break;
                          case CHUNK_Charset_to_use:
                              Page.CharsetName = ReadString(Stream, ChunkInfo.Length);
                              break;
                          case CHUNK_Graphic_to_use:
                               Page.CharsetID = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_Facing_direction:
                               Page.Facing_direction = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_Transparency:
                               Page.Transparency  = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_Movement_type:
                               Page.Movement_type  = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_Movement_frequency:
                               Page.Movement_frequency  = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_Activation_condition:
                               Page.Activation_condition  = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_Event_height:
                               Page.Event_height  = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_Prevent_event_overlap:
                               Page.event_overlap  = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_Animation_type:
                               Page.Animation_type  = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_Movement_speed:
                               Page.Movement_speed  = ReadCompressedInteger(Stream);
                               break;
                         case  CHUNK_Movement_block:
                              PageMovesChunk(Stream);
                              break;
		                  case CHUNK_Script_header:
                               Page.Script_header  = ReadCompressedInteger(Stream);
                               break;
                          case  CHUNK_Script_code:
                               Page.vcEvent_comand =  Event_parser.EventcommandChunk(Stream);
                                break;
                          case CHUNK_MAP_END_OF_BLOCK:
                               break;
                          default:
                                while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
                               break;
                          }
                } while(ChunkInfo.ID!=0);
          datareaded++;
          vcPage.push_back(Page);
          Page.clear();
          }
          ChunkInfo.ID =1;//para que no finalize
          return(vcPage);
}

std:: vector <stEventMap> map_reader::eventChunk(FILE * Stream)
{
         stEventMap Event;
         std:: vector <stEventMap> vcEvents;
         Event.clear();
         int id,datatoread=0,datareaded=0;
         string name;
         datatoread=ReadCompressedInteger(Stream);//numero de datos
         while(datatoread>datareaded) // si no hay mas en el array
         {
         id= ReadCompressedInteger(Stream);//lectura de id 1 de array
         do {
                         ChunkInfo.ID     = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
                         if(ChunkInfo.ID!=0)// si es fin de bloque no leas la longitud
                              ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño
                         switch(ChunkInfo.ID)// tipo de la primera dimencion
                         {
                          case CHUNK_EVENT_NAME:
                               Event.EventName = ReadString(Stream, ChunkInfo.Length);
                               break;
                          case CHUNK_EVENT_X:
                               Event.X_position         = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_EVENT_Y:
                               Event.Y_position         = ReadCompressedInteger(Stream);
                               break;
                          case CHUNK_EVENT_PAGES:
                               Event.vcPage =pageChunk(Stream);
                               break;
                          case CHUNK_MAP_END_OF_BLOCK:
                               break;
                          default:
                                while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
                               break;
                          }
           }while(ChunkInfo.ID!=0);
          datareaded++;
          vcEvents.push_back(Event);
          Event.clear();
          }
	return(vcEvents);
}

    void map_reader::GetNextChunk(FILE * Stream,map_data * data)
    {
        tChunk ChunkInfo; // informacion del pedazo leido
        unsigned char Void;

        // Loop while we haven't reached the end of the file
        while(!feof(Stream))
        {
            ChunkInfo.ID     = ReadCompressedInteger(Stream); // lectura de tipo del pedazo
            ChunkInfo.Length = ReadCompressedInteger(Stream); // lectura de su tamaño

            switch(ChunkInfo.ID)// segun el tipo
            {
                case CHUNK_MAP_CHIPSET:
                    data->ChipsetID        = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_WIDTH:
                    data->MapWidth         = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_HEIGHT:
                    data->MapHeight        = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_TYPE_OF_LOOP:
                    data->TypeOfLoop       = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_PARALLAX_BACK:
                    data->ParallaxBackground = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_BACKGROUND:
                    data->BackgroundName   = ReadString(Stream, ChunkInfo.Length);
                    break;

                case CHUNK_MAP_HORIZONTAL_PAN:
                    data->HorizontalPan    = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_VERTICAL_PAN:
                    data->VerticalPan      = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_HORIZONTAL_PAN_A:
                    data->HorizontalAutoPan = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_HORIZONTAL_PAN_SP:
                    data->HorizontalPanSpeed = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_VERTICAL_PAN_A:
                    data->VerticalAutoPan  = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_VERTICAL_PAN_SP:
                    data->VerticalPanSpeed = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_USE_GENERATOR: //0x2A   usar generdor aleatorio
                    data->use_genertor = ReadCompressedInteger(Stream);
                    break;
                case CHUNK_MAP_GEN_MODE: //0x2A    estilo de generacion
                    data->gen_mode = ReadCompressedInteger(Stream);
                    break;
                    //case 42: //0x2A npi (no poseo informacion)
                case CHUNK_MAP_GEN_NUM_TITLES: //0x32   granularidad
                    data->gen_Num_titles = ReadCompressedInteger(Stream);
                    break;
                case CHUNK_MAP_GEN_WIDTH: //0x31    largo
                    data->gen_width = ReadCompressedInteger(Stream);
                    break;
                case CHUNK_MAP_GEN_HEIGTH: //0x32    alto
                    data->gen_height = ReadCompressedInteger(Stream);
                    break;
                case CHUNK_MAP_GEN_SURROUND_MAP: //0x32    rodear mapa de titles adicionales
                    data->gen_surround_map = ReadCompressedInteger(Stream);
                    break;
                case CHUNK_MAP_GEN_UPER_WALL: //0x32   usar pared superior
                    data->gen_use_upper_wall = ReadCompressedInteger(Stream);
                    break;
                case CHUNK_MAP_GEN_FLOOR_B: //0x32   usar suelo b
                    data->gen_use_floor_b = ReadCompressedInteger(Stream);
                    break;
                case CHUNK_MAP_GEN_FLOOR_C: //0x32    usar suelo c
                    data->gen_use_floor_c = ReadCompressedInteger(Stream);
                    break;
                case CHUNK_MAP_GEN_EXTRA_B: //0x32   usar añadidos b
                    data->gen_use_extra_b = ReadCompressedInteger(Stream);
                    break;
                case CHUNK_MAP_GEN_EXTRA_C: //0x32  usar añadidos c
                    data->gen_use_extra_c = ReadCompressedInteger(Stream);
                    break;
               case CHUNK_MAP_GEN_X_POS: //0x3C  9 datos estandar  4 bytes X techo, pared_inferior,pared_superior, suelo_a , suelo_b, suelo_c, añadidos_a,añadidos_b,añadidos_c
                     fread(&data->gen_roof_X, sizeof(int), 1, Stream);
                     fread(&data->gen_down_wall_X, sizeof(int), 1, Stream);
                     fread(&data->gen_upper_wall_X, sizeof(int), 1, Stream);
                     fread(&data->gen_floor_a_X, sizeof(int), 1, Stream);
                     fread(&data->gen_floor_b_X, sizeof(int), 1, Stream);
                     fread(&data->gen_floor_c_X, sizeof(int), 1, Stream);
                     fread(&data->gen_extra_a_X, sizeof(int), 1, Stream);
                     fread(&data->gen_extra_b_X, sizeof(int), 1, Stream);
                     fread(&data->gen_extra_c_X, sizeof(int), 1, Stream);
                         break;
                case CHUNK_MAP_GEN_Y_POS: //0x3D  datos estandar  4 bytes  Y techo, pared_inferior,pared_superior, suelo_a , suelo_b, suelo_c, añadidos_a,añadidos_b,añadidos_c
                     fread(&data->gen_roof_Y, sizeof(int), 1, Stream);
                     fread(&data->gen_down_wall_Y, sizeof(int), 1, Stream);
                     fread(&data->gen_upper_wall_Y, sizeof(int), 1, Stream);
                     fread(&data->gen_floor_a_Y, sizeof(int), 1, Stream);
                     fread(&data->gen_floor_b_Y, sizeof(int), 1, Stream);
                     fread(&data->gen_floor_c_Y, sizeof(int), 1, Stream);
                     fread(&data->gen_extra_a_Y, sizeof(int), 1, Stream);
                     fread(&data->gen_extra_b_Y, sizeof(int), 1, Stream);
                     fread(&data->gen_extra_c_Y, sizeof(int), 1, Stream);
                    break;
                case CHUNK_MAP_GEN_CHIPSETS_IDS: //0x3E   //2bytes por dato, id de cada chipset, mismo formato que en layers
                   data->gen_chipset_ids = new unsigned short[ChunkInfo.Length>>1];
                    fread(data->gen_chipset_ids, sizeof(char), ChunkInfo.Length, Stream);
                    break;
                case CHUNK_MAP_LOWER_LAYER:
                    // Allocate lower map layer
                    data->LowerLayer = new unsigned short[ChunkInfo.Length>>1];
                    fread(data->LowerLayer, sizeof(char), ChunkInfo.Length, Stream);
                    break;

                case CHUNK_MAP_UPPER_LAYER:
                    // Allocate upper map layer
                    data->UpperLayer = new unsigned short[ChunkInfo.Length>>1];
                    fread(data->UpperLayer, sizeof(char), ChunkInfo.Length, Stream);
                    break;
                case CHUNK_MAP_EVENTS_LAYER:
                  data->vcEvents= eventChunk(Stream);
                   break;

                case CHUNK_MAP_TIMES_SAVED:
                    data->TimesSaved = ReadCompressedInteger(Stream);
                    break;

                case CHUNK_MAP_END_OF_BLOCK:
                    return;


                default:
                    // saltate un pedazo del tamaño de la longitud
                    while(ChunkInfo.Length--) fread(&Void, sizeof(char), 1, Stream);
                    break;
            }
        }
    }

    bool map_reader::Load(string Filename,map_data * data)
    {
	// Open map file to read
	    FILE * Stream;// apertura de archivo
        Stream = fopen(Filename.c_str(), "rb");
        if (Stream == NULL)
        {
            std::cerr << "Couldn't find LMU map: " << Filename << std::endl;
            exit(1);
        }
        string Header = ReadString(Stream); // lectura de cabezera
        if (Header != "LcfMapUnit") // comparacion con cabezera del mapa
        {// si no concuerda imprime un error y finaliza
            printf("Reading error: File is not a valid RPG2000 map\n");
            fclose(Stream);
            return false;
        }
        // Set default data of the map
        data->ChipsetID = 1;
        data->MapWidth = 20;
        data->MapHeight = 15;
        data->TypeOfLoop = 0;
        data->ParallaxBackground = false;
        data->BackgroundName = "None";
        data->HorizontalPan = false;
        data->HorizontalAutoPan = false;
        data->HorizontalPanSpeed = 0;
        data->VerticalPan = false;
        data->VerticalAutoPan = false;
        data->VerticalPanSpeed = 0;
        data->LowerLayer = NULL;
        data->UpperLayer = NULL;
        data->NumEvents = 0;
        GetNextChunk(Stream,data);// Get data from map
        fclose(Stream);
        return (true); // Done
    }

    void map_reader::ShowInformation(map_data * data)
    {
        printf("Map information\n"
               "===========================================================\n");
        printf("Chipset : %i\n", data->ChipsetID);
        printf("Size : %ix%i\n", data->MapWidth, data->MapHeight);
        printf("Type of Loop : %i\n", data->TypeOfLoop);
        printf("Use parallax background : %i\n", data->ParallaxBackground);
        if (data->ParallaxBackground)
        {
            printf(" -> Background file : %s\n", (data->BackgroundName).c_str());
            printf(" -> Horizontal pan : %i\n", data->HorizontalPan);
            if (data->HorizontalPan)
            {
                printf(" --> Horizontal auto pan : %i\n", data->HorizontalAutoPan);
                if (data->HorizontalAutoPan)
                    printf(" ---> Horizontal pan speed : %i\n", data->HorizontalPanSpeed);
            }
            printf(" -> Vertical pan : %i\n", data->VerticalPan);
            if (data->VerticalPan)
            {
                printf(" --> Vertical auto pan : %i\n", data->VerticalAutoPan);
                if (data->VerticalAutoPan)
                    printf(" ---> Vertical pan speed : %i\n", data->VerticalPanSpeed);
            }
        }

        if (data->LowerLayer != NULL)
        {
            printf("\nLower layer map data :\n");
            for (int y = 0; y < data->MapHeight; y++ )
            {
                for (int x = 0; x < data->MapWidth; x++ )
                    printf("%04X, ", data->LowerLayer[x+y*(data->MapWidth)]);
                printf("\n");
            }
        }
        if (data->UpperLayer != NULL)
        {
            printf("\nUpper layer map data :\n");
            for (int y = 0; y < data->MapHeight; y++ )
            {
                for (int x = 0; x < data->MapWidth; x++ )
                    printf("%04X, ", data->UpperLayer[x+y*(data->MapWidth)]);
                printf("\n");
            }
        }
        printf("Number of times saved : %i\n", data->TimesSaved);
        int x=(data->vcEvents).size();

        for (int y = 0; y < x; y++ )
        {
            data->vcEvents[y].show();
        }
    }

