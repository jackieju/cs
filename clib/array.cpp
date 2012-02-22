#include "array.h"
using namespace JUJU;
	void Array::add(void *p){
		if (used >= _size){
			enlarge();
		}
		fprintf(stderr, "-----used=%d\n", used);
		data[used] = p;
		used ++;
	}