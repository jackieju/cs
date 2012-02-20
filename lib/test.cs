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
load nanohttp;

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
//char* ss="fff1111111";
char* ss; 
ss = s;
 put_str(ss);
}
void tttt(){
 char* ss="fff1111111";

 put_str(ss);

}
void puts2(s1, s2){
char* ss1 = s1;
char* ss2 = s2;
	put_str(ss1);
	put_str(ss2);
}

void set(var name, var obj){
	// instance member
//	this->{name} = obj;  // variable named by value of name
//	this->name = obj;   // variable named 'name'
	// static variable
//	::name = obj;       
//	var a;
//	puts("=====  set ===>>>");
//	$.a = name;
	//char* n = "```````````````\n";
	//put_str(n);
//	puts(obj);
//	puts("#{name}=#{value}\r\n"");

}

/********* NOTICE ********************
 * test test1 test0 is pubfunction
*************************************/
void test_1(){
//var a = "adfafafasfas";
//puts(a);
//puts("sssssssssss");
puts2("ssssssssss1", "ttttttttttt2");
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

void testobj(){

	$.a = "990";
	char* aa = $.a;
	put_str(aa);
	puts($.a);
	
	$.b = 111111;
	int bb = $.b;
	putl(bb);
	
	$.b = $.a;
	puts($.b);
	
	var a;
	a.q = "qqqqqq";
	puts(a.q);
	
	a.q.qq = "qqqqqqq2";
	puts(a.q.qq);
	
	var test;
}

void test_basic(){
	
}

void test_hash(){
var b = {};	
var p = "u123";
var a = {
	{p}:"111",
	q: "faf",
	p: 32323
};
puts(a.u123);
puts(a.q);
int l = a.p;
putl(l);

}

void test_magic(){
var b = "aa";
var b2 = "9";
var a;
a.b = b;
a.{b} = "9999999999";
a.{b2} = "8888888888";
puts(a.{3*3}); // 8888888888
puts(a.aa); // 9999999999
}
void create()
{
//testobj();


//test_1();
//test2();
//test3();

//test_hash();
test_magic();



       // set("short", "东门\n");
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

	int server = create_server(9000);
	//putl(server);
	//server = testre();
	//server = 908990;
	int l = server;
	putl(server);
	addHandler(server, "*", "http/http");
      
}



};