#include "operator.hh"
#include <boost/bimap.hpp>
#include <boost/bimap/bimap.hpp>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

static std::unordered_map<std::string, Operator> OPERATOR_TABLE = {
    {"+", Operator::PLUS}, {"-", Operator::MINUS}, {"*", Operator::STAR}, {"/", Operator::SLASH}, {"<", Operator::LESS_THAN}, {"<=", Operator::LESS_EQUAL}, {">", Operator::GREATER_THAN}, {">=", Operator::GREATER_EQUAL}, {"==", Operator::EQUAL}, {"!=", Operator::NOT_EQUAL}, {":", Operator::COMPOUND}, {"=", Operator::EQ}};

const Operator& convertStringToOperator(const std::string& str)
{
    return OPERATOR_TABLE.at(str);
}

std::ostream& operator<<(std::ostream& out, const Operator& op)
{
    auto it = std::find_if(OPERATOR_TABLE.begin(), OPERATOR_TABLE.end(), [&op](const std::pair<std::string, Operator>& pair) {
        return pair.second == op;
    });

    if (it == OPERATOR_TABLE.end()) {
        throw std::invalid_argument("Operatore non valido.");
    }

    out << it->first;

    return out;
}