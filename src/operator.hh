#ifndef OPERATOR_HH
#define OPERATOR_HH
#include <iostream>
#include <string>

enum class Operator
{
    PLUS,
    MINUS,
    STAR,
    SLASH,
    LESS_THAN,
    LESS_EQUAL,
    GREATER_THAN,
    GREATER_EQUAL,
    EQUAL,
    NOT_EQUAL,
    COMPOUND,
    EQ
};

const Operator& convertStringToOperator(const std::string&);

std::ostream& operator<<(std::ostream&, const Operator&);

#endif