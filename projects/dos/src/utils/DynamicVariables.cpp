/*
 * DynamicVariables.cpp
 *
 *  Created on: Feb 25, 2009
 *      Author: k
 */

#include "DynamicVariables.hpp"
#include "oops/Exception.hpp"
#include "globals.hpp"
#include "utils/logger.hpp"

using namespace dos::utils;

void DynamicVariables::set(Name n, ValueType type, void * pVal){


	Value &v = m_map[n];

	//Delete the old value
	switch(type){
	case vtNone:
		break;
	case vtString:
		delete v.val.str;
		v.val.str = 0;
		break;
	case vtUser:

		break;
	};

	m_map[n].type = type;
	switch(type){
	case vtNone:
		break;
	case vtString:
		v.val.str = new std::string();
		*(v.val.str) = *(std::string *)pVal;
		break;
	case vtUser:
		v.val.usr = pVal;
		break;
	}
}

void DynamicVariables::clean(Value * v){
	switch(v->type){
	case vtNone:
		break;
	case vtString:
		delete v->val.str;
		v->val.str = 0;
		break;
	case vtUser:
		break;
	}

}

DynamicVariables::~DynamicVariables(){
	MapType::iterator it, itEnd;
	it = m_map.begin();
	itEnd = m_map.end();

	while(it != itEnd){
		Value &v = (it->second);
		clean(&v);
		it++;
	}
}


std::string * DynamicVariables::getString(Name n){
	if(m_map[n].type == vtString){
		return (m_map[n].val.str);
	}
	return 0;
}
