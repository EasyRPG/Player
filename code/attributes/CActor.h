#ifndef CACTOR_H
#define CACTOR_H

struct mot
{
	sll distance; 
	sll delta;
	unsigned char direction;
};


class CActor:public Chara {
// Methods
public:
	void MoveOnInput(); 
	int Min(int value, int max);
	sll Minf(float value, float max);
	int Clamp(int value, int min, int max);
	sll Clampf(float value, float min, float max);
	void setposXY(int x,int y);
	unsigned char  flags;
	unsigned char state;
	mot Cmotion;
	int GridX;///aparte de la X  y Y  tenemos la poscion con referencia bloques.
	int GridY;
	sll realX;
	sll realY;
	bool outofarea;
};

#endif /* CActor */
