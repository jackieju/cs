#ifndef _CLIB_ARRAY
#define _CLIB_ARRAY
#include <stdio.h>
#include <memory.h>
namespace JUJU{
	
class Array{
	
private:
public:
	void **data;
	int _size;
	int used;
	int _enlarge_size;
	void enlarge(){
				printf("///////!!!!!!!!!??????????////////////\n");
			fprintf(stderr, "-----dd25\n");
		void** new_data = new void*[_size+_enlarge_size];
		memset(new_data, 0, (_size+_enlarge_size)*sizeof(void*));
		if (data){
			memcpy(new_data, data, sizeof(void*)*used);
			delete data;
		}
		data = new_data;
		_size += _enlarge_size;
	}
	void enlargeTo(int s){
				printf("///////!!!!!!!!!??????????////////////\n");
		if (s <= _size)
			return;
		if (s - _size < _enlarge_size)
			s = _size + _enlarge_size;
		void** new_data = new void*[s];
		memset(new_data, 0, (s)*sizeof(void*));
		if (data){
			memcpy(new_data, data, (sizeof(void*))*used);
			delete data;
		}
		data = new_data;
		_size = s;
	}
public:
	
	void add(void *p);
	
	void setEnlargeSize(int s){
				printf("///////!!!!!!!!!??????????////////////\n");
		_enlarge_size = s;
	}
	
	
	int size(){
				printf("call size()\n");
		return used;
	}
	
	void* get(int i){
				printf("///////!!!!!!!!!??????????////////////\n");
		if (i >= _size)
			return NULL;
		return data[i];
	}
	void set(int i, void* p){
				printf("///////!!!!!!!!!??????????////////////\n");
	    if (i > _size){
			enlargeTo(i+1);
		}
		
		data[i] = p;
	}
    void*& operator[](int i){
		printf("///////!!!!!!!!!????operator[]??????////////////\n");
		printf("%d %d", i, _size);
		//void * rR = NULL;
		//return rR;
		if (i >= _size){
			enlargeTo(i+1);
		}
		
		void** r = (void**)&(data[i]);
		//printf("----->ppppp=data=%x\n", &data[i]);
		return data[i];
	}
	Array(){
				printf("///////!!!!!!!!!?????Array()?????////////////\n");
		data = NULL;
		used = 0;
		_size = 0;
		_enlarge_size = 1024;
	};
	Array(int size){
				printf("///////!!!!!!!!!??????????////////////\n");
		data = new void *[size];
		memset(data, 0, (sizeof(void*))*size);
		used = 0;
		_size = size;
		
	}
	~Array(){
				printf("///////!!!!!!!!!??????????////////////\n");
		for (int i = 0; i<used; i++){
			if (data[i]){
				delete data[i];
				data[i]=NULL;
			}
		}
		if (data)
			delete data;
		
	};
	
};

}; // namespace
#endif


