#ifndef CDELTATIME_H
#define CDELTATIME_H

class CDeltaTime {
    public:
        // Methods
	CDeltaTime(int pIdealFPS);
        void setIdealFPS(int pIdealFPS);
 	void clear(); 
	void update();
        // Attributes
        float deltaTime;
    private:
        long  idealFPS;
        float idealTime;
        float  timePrevious;
        float  timeCurrent;      

        float deltaTimeArray[16];
        int  deltaCurrentVector;
};
#endif /* CDeltaTime */
