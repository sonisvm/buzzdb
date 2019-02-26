#include "date_field.h"
#include <sstream>
#include <iostream>
#include <iomanip>

namespace emerald
{ 
    DateField::DateField(std::string v): Field(field_type::DATE){
        std::string date_format = "%m/%d/%y";
        std::istringstream ss(v);
        std::tm dt;
        ss >> std::get_time(&dt, date_format.c_str());
        this->value = std::mktime(&dt);
    };

    void DateField::print() const{
        std::tm dt = *std::localtime(&value);
        std::stringstream wss;
        wss << (std::put_time(&dt, "%m/%d/%y"));
        std::cout << wss.str() << " \n" ;
    };

    std::time_t DateField::getValue() const{
        return this->value;
    };

    bool DateField::filter(Predicate::opType op, Field* value){
        //std::cout << "Calling datefield filter\n";
        DateField* date_value = static_cast<DateField*>(value);
        //std::cout << date_value->getValue()<< " " << this->value <<"\n";
        switch (op)
        {
            case Predicate::opType::EQ :
                return std::difftime(this->value, date_value->getValue())==0;
                break;
            case Predicate::opType::NE :
                return std::difftime(this->value, date_value->getValue())!=0;
                break;
            case Predicate::opType::GT :
                return std::difftime(this->value, date_value->getValue())>0;
                break;
            case Predicate::opType::LT :
                return std::difftime(this->value, date_value->getValue())<0;
                break;
            case Predicate::opType::GE :
                return std::difftime(this->value, date_value->getValue())>=0;
                break;
            case Predicate::opType::LE :
                return std::difftime(this->value, date_value->getValue())<=0;
                break;
            default:
                return false;
                break;
        } 
    };

    DateField::DateField(const DateField& field):Field(field_type::DATE){
        value = field.getValue();
    };
} // emerald