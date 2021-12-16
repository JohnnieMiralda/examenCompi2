#include "ast.h"
#include <iostream>
#include <sstream>
#include <set>
#include "asm.h"
#include <map>

const char * floatTemps[] = {"$f0","$f1","$f2","$f3","$f4","$f5","$f6","$f7","$f8","$f9","$f10","$f11","$f12","$f13","$f14","$f15","$f16","$f17","$f18","$f19","$f20","$f21","$f22","$f23","$f24","$f25","$f26","$f27","$f28","$f29","$f30","$f31"
                        };

#define FLOAT_TEMP_COUNT 32
set<string> intTempMap;
set<string> floatTempMap;

extern Asm assemblyFile;


map<string, int> codeGenerationVars;

int globalStackPointer = 0;

string getFloatTemp(){
    for (int i = 0; i < FLOAT_TEMP_COUNT; i++)
    {
        if(floatTempMap.find(floatTemps[i]) == floatTempMap.end()){
            floatTempMap.insert(floatTemps[i]);
            return string(floatTemps[i]);
        }
    }
    cout<<"No more float registers!"<<endl;
    return "";
}

void releaseFloatTemp(string temp){
    floatTempMap.erase(temp);
}

void FloatExpr::genCode(Code &code){
    string floatTemp = getFloatTemp();
    code.place = floatTemp;
    stringstream ss;
    ss << "li.s " << floatTemp <<", "<< this->number <<endl;
    code.code = ss.str();
}

void SubExpr::genCode(Code &code){
    stringstream ss;
    ss<<code.code<<endl;
    Code leftExpr;
    Code rightExpr;
    this->expr1->genCode(leftExpr);
    this->expr2->genCode(rightExpr);
    string temp= getFloatTemp();
    ss<<leftExpr.code<<endl;
    ss<<rightExpr.code<<endl;
    ss<<"sub.s "<<temp<<" , "<<leftExpr.place<<" , "<< rightExpr.place<<endl;
    releaseFloatTemp(leftExpr.place);
    releaseFloatTemp(rightExpr.place);
    code.place= temp;
    code.code= ss.str();
}

void DivExpr::genCode(Code &code){
    stringstream ss;
    ss<<code.code<<endl;
    Code leftExpr;
    Code rightExpr;
    this->expr1->genCode(leftExpr);
    this->expr2->genCode(rightExpr);
    string temp= getFloatTemp();
    ss<<leftExpr.code<<endl;
    ss<<rightExpr.code<<endl;
    ss<<"div.s "<<temp<<" , "<<leftExpr.place<<" , "<< rightExpr.place<<endl;
    releaseFloatTemp(leftExpr.place);
    releaseFloatTemp(rightExpr.place);
    code.place= temp;
    code.code= ss.str();
}

void IdExpr::genCode(Code &code){
    string temp= getFloatTemp();
    stringstream ss;
    ss<<code.code<<endl;
    ss<< "l.s "<< temp <<", "<<codeGenerationVars[id]<<"($sp)"<<endl;
    code.code= ss.str();
    code.place= temp;
}

string ExprStatement::genCode(){
    Code exp;
    this->expr->genCode(exp);
    releaseFloatTemp(exp.place);
    return exp.code;
}

string IfStatement::genCode(){
    return "If statement code generation\n";
}

void MethodInvocationExpr::genCode(Code &code){
    
}

string AssignationStatement::genCode(){
    stringstream ss;
    Code value;
    this->value->genCode(value);
    ss<<value.code<<endl;
    codeGenerationVars[id]=globalStackPointer;
    globalStackPointer+=4;
    ss<< "s.s "<< value.place <<", "<<codeGenerationVars[id]<<"($sp)"<<endl;
    releaseFloatTemp(value.place);
    return ss.str();
}

void GteExpr::genCode(Code &code){
    stringstream ss;
    ss<<code.code<<endl;
    Code leftExpr;
    Code rightExpr;
    this->expr1->genCode(leftExpr);
    this->expr2->genCode(rightExpr);
    string temp= getFloatTemp();
    ss<< "c.lt.s "<< rightExpr.place<< ", "<< leftExpr.place<<endl;
    releaseFloatTemp(leftExpr.place);
    releaseFloatTemp(rightExpr.place);
    code.place= temp;
    code.code= ss.str();
}

void LteExpr::genCode(Code &code){
    stringstream ss;
    ss<<code.code<<endl;
    Code leftExpr;
    Code rightExpr;
    this->expr1->genCode(leftExpr);
    this->expr2->genCode(rightExpr);
    string temp= getFloatTemp();
    ss<< "c.lt.s "<< leftExpr.place<< ", "<< rightExpr.place<<endl;
    releaseFloatTemp(leftExpr.place);
    releaseFloatTemp(rightExpr.place);
    code.place= temp;
    code.code= ss.str();
}

void EqExpr::genCode(Code &code){
    stringstream ss;
    ss<<code.code<<endl;
    Code leftExpr;
    Code rightExpr;
    this->expr1->genCode(leftExpr);
    this->expr2->genCode(rightExpr);
    string temp= getFloatTemp();
    ss<<"c.eq.s "<<leftExpr.place<<" , "<< rightExpr.place<<endl;
    releaseFloatTemp(leftExpr.place);
    releaseFloatTemp(rightExpr.place);
    code.place= temp;
    code.code= ss.str();
}

void ReadFloatExpr::genCode(Code &code){
    stringstream ss;
    ss <<code.code<<endl;
    ss<<"li $v0, 6"<<endl
    <<"syscall"<<endl;
    code.code = ss.str();
}

string PrintStatement::genCode(){
    Code exp;
    stringstream ss;
    list<Expr *>::iterator itd = this->expressions.begin();
    while (itd != this->expressions.end()){
        Expr * dec = *itd;
        dec->genCode(exp);
        ss<<exp.code<<endl;
        releaseFloatTemp(exp.place);
    }
    releaseFloatTemp(exp.place);

    ss<<exp.code<<endl;  
    ss<<"mov.s $f12, "<<exp.place<<endl
    <<"li $v0, 2"<<endl
    <<"syscall"<<endl;

    return ss.str();
}

string ReturnStatement::genCode(){
    Code expr;
    this->expr->genCode(expr);
    releaseFloatTemp(expr.place);
    stringstream ss;
    ss<<expr.code<<endl;
    ss<<"mfc1 $v0, "<<expr.place<<endl;
    return ss.str();
}

string MethodDefinitionStatement::genCode(){
    return "Method definition code generation\n";
}