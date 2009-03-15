#include "Move_management.h"

void  Mv_management::init(General_data * TheTeam)
{
	Charas_nps=&(TheTeam->GCharas_nps);//agregar apuntador a vector de eventos
	Actor=TheTeam->Players.get_chara(0);
	chip=&(TheTeam->Gchip);
	data=&(TheTeam->Gdata);
}


bool Mv_management::npc_colision(int x, int y,int e)
{
    unsigned int i;
    for (i = 0; i < Charas_nps->size(); i++)
    {
        if (i!=(unsigned)e)
            if (( Charas_nps->at(i).GridX==x) &&(Charas_nps->at(i).GridY==y))
                //if(Charas_nps->at(i).layer==1)
                return(false);
    }
    if (( Actor->GridX==x) &&(Actor->GridY==y))
        return(false);
    return(true);
}

int Mv_management::random_move(int i)
{
    int temp,z;
    temp=rand()%4;
        for (z=0;z<4;z++)
        {
            switch (temp)
            {
            case DIRECTION_UP:
                if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_UP))&&(npc_colision(Charas_nps->at(i).GridX, (Charas_nps->at(i).GridY-1),i)))
                    return(temp);
                break;
            case DIRECTION_DOWN:
                if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_DOWN))&&(npc_colision(Charas_nps->at(i).GridX, (Charas_nps->at(i).GridY+1),i)))
                    return(temp);
                break;
            case DIRECTION_LEFT:
                if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_LEFT))&&(npc_colision((Charas_nps->at(i).GridX-1), Charas_nps->at(i).GridY,i)))
                    return(temp);
                break;
            case DIRECTION_RIGHT:
                if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_RIGHT))&&(npc_colision((Charas_nps->at(i).GridX+1), Charas_nps->at(i).GridY,i)))
                    return(temp);
                break;
            }
            temp=(temp+1)%4;
        }
        return(5);
}

int Mv_management::up_down(int i)
{
        if (Charas_nps->at(i).move_dir==0x00)
        {
            if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_UP))&&(npc_colision(Charas_nps->at(i).GridX, (Charas_nps->at(i).GridY-1),i)))
                return(0x00);
            else
                if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_DOWN))&&(npc_colision(Charas_nps->at(i).GridX, (Charas_nps->at(i).GridY+1),i)))
                    return(0x01);
        }
        else
        {
            if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_DOWN))&&(npc_colision(Charas_nps->at(i).GridX, (Charas_nps->at(i).GridY+1),i)))
                return(0x01);
            else
                if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_UP))&&(npc_colision(Charas_nps->at(i).GridX, (Charas_nps->at(i).GridY-1),i)))
                    return(0x00);
        }
        return(5);//default
}

int Mv_management::left_right(int i)
{
  if (Charas_nps->at(i).move_dir==0x03)
        {
            if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_RIGHT))&&(npc_colision((Charas_nps->at(i).GridX+1), Charas_nps->at(i).GridY,i)))
                return(0x03);
            else
                if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_LEFT))&&(npc_colision((Charas_nps->at(i).GridX-1), Charas_nps->at(i).GridY,i)))
                    return(0x02);
        }
        else
        {
            if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_LEFT))&&(npc_colision((Charas_nps->at(i).GridX-1), Charas_nps->at(i).GridY,i)))
                return(0x02);
            else
                if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_RIGHT))&&(npc_colision((Charas_nps->at(i).GridX+1), Charas_nps->at(i).GridY,i)))
                    return(0x03);
        }
        return(5);//default
}

int Mv_management::to_hero(int i)
{

    int temp,z;

    if (Charas_nps->at(i).GridX<Actor->GridX)
        {
            if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_RIGHT))&&(npc_colision((Charas_nps->at(i).GridX+1), Charas_nps->at(i).GridY,i)))
                return(0x03);
        }
        if (Charas_nps->at(i).GridX>Actor->GridX)
        {
            if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_LEFT))&&(npc_colision((Charas_nps->at(i).GridX-1), Charas_nps->at(i).GridY,i)))
                return(0x02);
        }
        if (Charas_nps->at(i).GridY<Actor->GridY)
        {
            if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_DOWN))&&(npc_colision(Charas_nps->at(i).GridX, (Charas_nps->at(i).GridY+1),i)))
                return(0x01);
        }
        if (Charas_nps->at(i).GridY>Actor->GridY)
        {
            if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_UP))&&(npc_colision(Charas_nps->at(i).GridX, (Charas_nps->at(i).GridY-1),i)))
                return(0x00);
        }
        temp=rand()%4;
        for (z=0;z<4;z++)
        {
            switch (temp)
            {
            case DIRECTION_UP:
                if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_UP))&&(npc_colision(Charas_nps->at(i).GridX, (Charas_nps->at(i).GridY-1),i)))
                    return(temp);
                break;
            case DIRECTION_DOWN:
                if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_DOWN))&&(npc_colision(Charas_nps->at(i).GridX, (Charas_nps->at(i).GridY+1),i)))
                    return(temp);
                break;
            case DIRECTION_LEFT:
                if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_LEFT))&&(npc_colision((Charas_nps->at(i).GridX-1), Charas_nps->at(i).GridY,i)))
                    return(temp);
                break;
            case DIRECTION_RIGHT:
                if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_RIGHT))&&(npc_colision((Charas_nps->at(i).GridX+1), Charas_nps->at(i).GridY,i)))
                    return(temp);
                break;
            }
            temp=(temp+1)%4;
        }
        return(5);
 }

int Mv_management::escape_hero(int i)
{
    int temp,z;

        if (Charas_nps->at(i).GridX<Actor->GridX)
        {
            if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_LEFT))&&(npc_colision((Charas_nps->at(i).GridX-1), Charas_nps->at(i).GridY,i)))
                return(0x02);

        }
        if (Charas_nps->at(i).GridX>Actor->GridX)
        {
            if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_RIGHT))&&(npc_colision((Charas_nps->at(i).GridX+1), Charas_nps->at(i).GridY,i)))
                return(0x03);
        }
        if (Charas_nps->at(i).GridY<Actor->GridY)
        {
            if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_UP))&&(npc_colision(Charas_nps->at(i).GridX, (Charas_nps->at(i).GridY-1),i)))
                return(0x00);
        }
        if (Charas_nps->at(i).GridY>Actor->GridY)
        {
            if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_DOWN))&&(npc_colision(Charas_nps->at(i).GridX, (Charas_nps->at(i).GridY+1),i)))
                return(0x01);
        }
        temp=rand()%4;
        for (z=0;z<4;z++)
        {
            switch (temp)
            {
            case DIRECTION_UP:
                if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_UP))&&(npc_colision(Charas_nps->at(i).GridX, (Charas_nps->at(i).GridY-1),i)))
                    return(temp);
                break;
            case DIRECTION_DOWN:
                if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_DOWN))&&(npc_colision(Charas_nps->at(i).GridX, (Charas_nps->at(i).GridY+1),i)))
                    return(temp);
                break;
            case DIRECTION_LEFT:
                if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_LEFT))&&(npc_colision((Charas_nps->at(i).GridX-1), Charas_nps->at(i).GridY,i)))
                    return(temp);
                break;
            case DIRECTION_RIGHT:
                if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_RIGHT))&&(npc_colision((Charas_nps->at(i).GridX+1), Charas_nps->at(i).GridY,i)))
                    return(temp);
                break;
            }
            temp=(temp+1)%4;
        }
        return(5);
}

int Mv_management::custom(int i)
{
        int temp;
        temp=data->vcEvents[i].vcPage[0].vcPage_Moves.vcMovement_commands[Charas_nps->at(i).actual_move].Comand;
        Charas_nps->at(i).actual_move= ((Charas_nps->at(i).actual_move+1)%data->vcEvents[i].vcPage[0].vcPage_Moves.Movement_length);
        switch (temp)
        {
        case 0:
            if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_UP))&&(npc_colision(Charas_nps->at(i).GridX, (Charas_nps->at(i).GridY-1),i)))
                return(DIRECTION_UP);
            break;
        case 2:
            if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_DOWN))&&(npc_colision(Charas_nps->at(i).GridX, (Charas_nps->at(i).GridY+1),i)))
                return(DIRECTION_DOWN);
            break;
        case 3:
            if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_LEFT))&&(npc_colision((Charas_nps->at(i).GridX-1), Charas_nps->at(i).GridY,i)))
                return(DIRECTION_LEFT);
            break;
        case 1:
            if ((chip->CollisionAt(Charas_nps->at(i).GridX,Charas_nps->at(i).GridY,DIRECTION_RIGHT))&&(npc_colision((Charas_nps->at(i).GridX+1), Charas_nps->at(i).GridY,i)))
                return(DIRECTION_RIGHT);
            break;
        }
    return(5);
}

int Mv_management::get_dir(int i)
{
    if (data->vcEvents[i].vcPage[0].Movement_type==0)//do not move
        return(5);
    if (data->vcEvents[i].vcPage[0].Movement_type==1)// random
    {
        return(random_move( i));
    }
    if (data->vcEvents[i].vcPage[0].Movement_type==2)// up down
    {
        return(up_down(i));
    }
    if (data->vcEvents[i].vcPage[0].Movement_type==3)// left right
    {
        return(left_right(i));
    }
    if (data->vcEvents[i].vcPage[0].Movement_type==4)// go to the hero
    {
        return(to_hero(i));
    }
    if (data->vcEvents[i].vcPage[0].Movement_type==5)// run frome the hero
    {
        return(escape_hero(i));
    }
    if (data->vcEvents[i].vcPage[0].Movement_type==6)// use the stack
    {
     return(custom(i));
    }

    return(5);
}

