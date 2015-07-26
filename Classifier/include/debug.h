/*
 *Debug.hpp
 *Made by Daiem
*/

#ifndef DEBUG_H
#define DEBUG_H

#ifndef DEBUGING
//#define DEBUGING
#endif

#include <stdio.h>
#include <iostream>
#include <fstream>

#define DEBUG_STREAM std::cout
#define DEBUG_LOG printf
#define ERROR_LOG printf

namespace local_dbg{

static std::streambuf *cout_buf;
static std::ofstream log_out_file;
static void redirect_outstream_file(){
	if(!log_out_file.is_open()){
		log_out_file.open("log.txt");
	}
	cout_buf = std::cout.rdbuf(); //save old buf
	std::cout.rdbuf(log_out_file.rdbuf()); //redirect std::cout to out.txt!
}

static void redirect_outstream_stdout(){
	std::cout.rdbuf(cout_buf);
//	log_out_file.close();
}

static void print_float_(float* array,int count){
	DEBUG_STREAM << "[";
	for(int i = 0; i < count; i++){
		DEBUG_STREAM << array[i];
		if(i != count-1)
			DEBUG_STREAM << ",";
	}
	DEBUG_STREAM << "]\n";
}
}

#endif // DEBUG_H
