#ifndef __scene__
#define __scene__

class Scene {
public:
	virtual ~Scene() {};

	virtual void main_function() =0;
};

#endif // __scene__
