/** @file udmfparser.cpp  UDMF parser.
 *
 * @authors Copyright (c) 2016-2017 Jaakko Keränen <jaakko.keranen@iki.fi>
 *
 * @par License
 * GPL: http://www.gnu.org/licenses/gpl.html
 *
 * <small>This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version. This program is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details. You should have received a copy of the GNU
 * General Public License along with this program; if not, see:
 * http://www.gnu.org/licenses</small>
 */

#include "udmfparser.h"

using namespace de;

UDMFParser::UDMFParser()
{}

void UDMFParser::setGlobalAssignmentHandler(UDMFParser::AssignmentFunc func)
{
    _assignmentHandler = func;
}

void UDMFParser::setBlockHandler(UDMFParser::BlockFunc func)
{
    _blockHandler = func;
}

UDMFParser::Block const &UDMFParser::globals() const
{
    return _globals;
}

void UDMFParser::parse(String const &input)
{
    // Lexical analyzer for Haw scripts.
    _analyzer = UDMFLex(input);

    while (nextFragment() > 0)
    {
        if (_range.lastToken().equals(UDMFLex::BRACKET_OPEN))
        {
            String const blockType = _range.firstToken().str().toLower();

            Block block;
            parseBlock(block);

            if (_blockHandler)
            {
                _blockHandler(blockType, block);
            }
        }
        else
        {
            parseAssignment(_globals);
        }
    }

    // We're done, free the remaining tokens.
    _tokens.clear();
}

dsize UDMFParser::nextFragment()
{
    _analyzer.getExpressionFragment(_tokens);

    // Begin with the whole thing.
    _range = TokenRange(_tokens);

    return _tokens.size();
}

void UDMFParser::parseBlock(Block &block)
{
    // Read all the assignments in the block.
    while (nextFragment() > 0)
    {
        if (_range.firstToken().equals(UDMFLex::BRACKET_CLOSE))
            break;

        parseAssignment(block);
    }
}

void UDMFParser::parseAssignment(Block &block)
{
    if (_range.isEmpty())
        return; // Nothing here?

    if (!_range.lastToken().equals(UDMFLex::SEMICOLON))
    {
        throw SyntaxError("UDMFParser::parseAssignment",
                          "Expected expression to end in a semicolon at " +
                          _range.lastToken().asText());
    }
    if (_range.size() == 1)
        return; // Just a semicolon?

    if (!_range.token(1).equals(UDMFLex::ASSIGN))
    {
        throw SyntaxError("UDMFParser::parseAssignment",
                          "Expected expression to have an assignment operator at " +
                          _range.token(1).asText());
    }

    String const identifier = _range.firstToken().str().toLower();
    Token const &valueToken = _range.token(2);

    // Store the assigned value into a variant.
    QVariant value;
    switch (valueToken.type())
    {
    case Token::KEYWORD:
        if (valueToken.equals(UDMFLex::T_TRUE))
        {
            value.setValue(true);
        }
        else if (valueToken.equals(UDMFLex::T_FALSE))
        {
            value.setValue(false);
        }
        else
        {
            throw SyntaxError("UDMFParser::parseAssignment",
                              "Unexpected value for assignment at " + valueToken.asText());
        }
        break;

    case Token::LITERAL_NUMBER:
        if (valueToken.isInteger())
        {
            value.setValue(valueToken.toInteger());
        }
        else
        {
            value.setValue(valueToken.toDouble());
        }
        break;

    case Token::LITERAL_STRING_QUOTED:
        value.setValue(QString(valueToken.unescapeStringLiteral()));
        break;

    case Token::IDENTIFIER:
        value.setValue(QString(valueToken.str()));
        break;

    default:
        DENG2_ASSERT(0!="[UMDFParser::parseAssignment] Invalid token type");
        break;
    }

    block.insert(identifier, value);

    if (_assignmentHandler && (&block == &_globals))
    {
        _assignmentHandler(identifier, value);
    }
}
