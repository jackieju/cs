
#ifdef _MACOS
#define __stdcall 
#endif

extern "C"{
void __stdcall put_str(char* msg);

}
class testg{
public:
	int a;
	int b;
	
	testg(){
		a = 0;
		b = 0;
	}
	~testg(){
		
	}
};

class testa{
public:

	testg *t;
	testa(){
		t = new testg();
	}
	~testa(){
		delete t;
	}
	void go(){
		int d = t->a;
		t->a = 10;
		printf("\nt.a=%d\n", t->a);
	}
};