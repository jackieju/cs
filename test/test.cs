// this source is to test the grammer and vm(interpretor)
//#include "test.h"
//

// import class def used by this class
/*use cgi;  
use object;
use a::testobj;
*/
// load dynamic link library
load baselib; 


//inherit object;
int a;
class test{
/*test comments*/
void main( j,  p)
{
	// b;
	var b = "daff";
	my a = "ha";
//	main("","");
	//var t = new a::testobj();
}

void puts(s){
//char* ss;
 //ss = s;
put_str(s);
}

void set(var name, var obj){
	// instance member
//	this->{name} = obj;  // variable named by value of name
//	this->name = obj;   // variable named 'name'
	// static variable
//	::name = obj;       
	var a;
	puts("=====----===>>>");
	//char* n = "```````````````\n";
	//put_str(n);
//	puts(obj);
//	puts("#{name}=#{value}\r\n"");
}

void test0(){

}


void test11( var a){
    putl(999);
}

void test_putl(var b){
	putl(b);
}

void test2(){

	int l = 1111111;
	putl(l);
	
	var a = 2222222;
	int l2 = a;
	putl(l2);
	
	int l3 = 3333333;
	var a2 = l3;
	int l1 = a2;
	putl(l1);
}

void test3(){
	char* p = "ppppp";
	put_str(p);
	
	var a = "qqqqqqqq";
	char* q = a;
	
	put_str(a);
	char* pq = "pqpqpqpqpqpq";
	
	var b = pq;
	char* pq1 = b;
	put_str(pq1);
}
void create()
{
test2();

//	a = l ;
	//put_str(a);
	//test_putl(a);

/*		int l = 110;
	put_str("============= test putl(int l) ===========\n");
	// test call pub function directly
	put_str("******");putl(l);put_str("\n");
	put_str("******");putl(324);put_str("\n");

	// test call pub function indirectly
	put_str("******");test11(112);put_str("\n");
	
	puts("ffasfasfafdakfjaslafsljafjafll............\n");

	test0();
*/
// = 123456;


/*	put_str("1234567890\r\nok\r\n");
	put_str("afa\r\n");
	test_putl(l);
        set("short", "东门\n");*/
    /*    set("long",
"这是东城门，城门正上方刻着“"+"东门"+"”两个楷书大字。城墙上贴着几张官府"+"告示(gaoshi)。官兵们警惕地注视着过往行人，好不威严。这里车水马龙，到处都是行人。虽然这里是城门口，但是小贩们也来到这里，不远处，有卖水的、卖苹果的、橘子香瓜、零食小点，真是要什么有什么。一条笔直的青石板大道向东西两边延伸。东边是郊外，北面有一片茂盛的青竹林。\n"
        );
        set("outdoors", "city");

      //  set("item_desc", {
       //         "gaoshi" :  set 
       // });
        set("exits", {
                "east" : "/d/taishan/yidao",
                "north" : "/d/gaibang/ml1",
                "west" : "dongdajie3",
                "south" : "dongjiao3"
        });

        set("objects", {
                "npc/wujiang" : 1,
                "npc/bing" : 2,
                "/clone/npc/zuo" : 1
        });

       set("cost", 1);*/
         //setup();
      
}



};