#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include<iostream>
#include<string>
using namespace std;

//ע������˳����������Զ���Ŀ��ļ���Ҫ�õ�����������ļ���һ��Ҫ��֮ǰ����
#include "uv.h"
#include "session.h"
#include "session_uv.h"

session* uv_session::create()
{
	uv_session*	uv_s = new uv_session(); //temp

	 return (session*)uv_s;
 }

void uv_session::destroy(uv_session* s)
{
	s->exit(); 

	delete s;  
}

void uv_session::init()
{
	memset(this->c_address, 0, sizeof(this->c_address));
	this->c_port = 0;
	this->recved = 0;
}

void uv_session::exit()
{

}

void uv_session::close()
{

}

void uv_session::send_data(unsigned char* body, int len)
{

}

const char* uv_session::get_address(int* client_port)
{

}