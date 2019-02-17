#pragma once

#include "poly_visitor.hpp"

class ASTNode;

class Expression;
class Variable;
class Literal;
class BinaryOperation;
class UnaryOperation;
class ArrayAccess;
class RecordAccess;
class FunctionCall;
class Cast;

class ASTNodeStmt;
class AssignmentStmt;
class FunctionCallStmt;
class BlockStmt;
class IfStmt;
class WhileStmt;
class ForStmt;
class BreakStmt;
class ContinueStmt;

class Symbol;

class SymType;
class Int;
class Double;
class Char;
class Boolean;
class TPointer;
class String;
class Void;
class Alias;
class Pointer;
class StaticArray;
class OpenArray;
class Record;
class FunctionSignature;
class ForwardType;

class SymVar;
class LocalVar;
class GlobalVar;
class ParamVar;
class Const;

class SymFun;
class ForwardFunction;
class Function;
class MainFunction;
class Read;
class Write;
class Trunc;
class Round;
class Succ;
class Prev;
class Chr;
class Ord;
class High;
class Low;
class Exit;

class Tables;
template <typename T>
class TableSymbol;

using base_visitor = poly_visitor::base_visitor<
		Variable, Literal, BinaryOperation, UnaryOperation,
		ArrayAccess, RecordAccess, FunctionCall, Cast,
		AssignmentStmt, FunctionCallStmt, BlockStmt, IfStmt, WhileStmt,
		ForStmt, BreakStmt, ContinueStmt,
		Int, Double, Char, Boolean, TPointer, String, Void, Alias,
		Pointer, StaticArray, OpenArray,  Record,
		FunctionSignature, ForwardType,
		LocalVar, GlobalVar, ParamVar, Const,
		ForwardFunction, Function, MainFunction,
		Read, Write, Trunc, Round, Succ, Prev, Chr, Ord, High, Low, Exit,
		TableSymbol<std::shared_ptr<SymType>>,
		TableSymbol<std::shared_ptr<SymVar>>,
		TableSymbol<std::shared_ptr<Const>>,
		TableSymbol<std::shared_ptr<SymFun>>,
		Tables>;
