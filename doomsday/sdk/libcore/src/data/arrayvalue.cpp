/*
 * The Doomsday Engine Project -- libcore
 *
 * Copyright © 2004-2017 Jaakko Keränen <jaakko.keranen@iki.fi>
 *
 * @par License
 * LGPL: http://www.gnu.org/licenses/lgpl.html
 *
 * <small>This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version. This program is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
 * General Public License for more details. You should have received a copy of
 * the GNU Lesser General Public License along with this program; if not, see:
 * http://www.gnu.org/licenses</small>
 */

#include "de/ArrayValue"
#include "de/FunctionValue"
#include "de/NumberValue"
#include "de/Process"
#include "de/Reader"
#include "de/RecordValue"
#include "de/TextValue"
#include "de/Writer"

#include <algorithm>
#include <QTextStream>

namespace de {

ArrayValue::ArrayValue() : Value(), _iteration(0)
{}

ArrayValue::ArrayValue(ArrayValue const &other) : Value(), _iteration(0)
{
    for (auto i : other._elements)
    {
        _elements.push_back(i->duplicate());
    }
}

ArrayValue::ArrayValue(StringList const &strings)
{
    for (String str : strings)
    {
        _elements.push_back(new TextValue(str));
    }
}

ArrayValue::ArrayValue(std::initializer_list<Value *> values) : _iteration(0)
{
    for (auto *v : values)
    {
        _elements.push_back(v);
    }
}

ArrayValue::~ArrayValue()
{
    clear();
}

Value *ArrayValue::duplicate() const
{
    return new ArrayValue(*this);
}

Value::Text ArrayValue::asText() const
{
    Text result;
    QTextStream s(&result);
    s << "[";

    bool isFirst = true;
    bool hadNewline = false;

    // Compose a textual representation of the array elements.
    for (Elements::const_iterator i = _elements.begin();
        i != _elements.end(); ++i)
    {
        String content = (*i)->asText();
        bool multiline = content.contains('\n');
        if (!isFirst)
        {
            if (hadNewline || multiline) s << "\n";
            s << ",";
        }
        hadNewline = multiline;
        s << " " << content.replace("\n", "\n  ");
        isFirst = false;
    }

    s << " ]";
    return result;
}

dsize ArrayValue::size() const
{
    return _elements.size();
}

Value const &ArrayValue::element(Value const &indexValue) const
{
    NumberValue const *v = dynamic_cast<NumberValue const *>(&indexValue);
    if (!v)
    {
        /// @throw IllegalIndexError @a indexValue is not a NumberValue.
        throw IllegalIndexError("ArrayValue::element", "Array index must be a number");
    }
    dint index = v->asInt();
    Elements::const_iterator elem = indexToIterator(index);
    return **elem;
}

Value &ArrayValue::element(Value const &index)
{
    return const_cast<Value &>(const_cast<ArrayValue const *>(this)->element(index));
}

void ArrayValue::setElement(Value const &indexValue, Value *value)
{
    NumberValue const *v = dynamic_cast<NumberValue const *>(&indexValue);
    if (!v)
    {
        /// @throw IllegalIndexError @a indexValue is not a NumberValue.
        throw IllegalIndexError("ArrayValue::setElement", "Array index must be a number");
    }
    replace(v->asInt(), value);
}

bool ArrayValue::contains(Value const &value) const
{
    for (Elements::const_iterator i = _elements.begin(); i != _elements.end(); ++i)
    {
        if (!(*i)->compare(value))
        {
            return true;
        }
    }
    return false;
}

Value *ArrayValue::begin()
{
    _iteration = 0;
    return next();
}

Value *ArrayValue::next()
{
    if (_iteration < dint(_elements.size()))
    {
        return _elements[_iteration++]->duplicate();
    }
    return 0;
}

bool ArrayValue::isTrue() const
{
    return _elements.size() > 0;
}

dint ArrayValue::compare(Value const &value) const
{
    ArrayValue const *other = dynamic_cast<ArrayValue const *>(&value);
    if (other)
    {
        if (size() < other->size())
        {
            return -1;
        }
        if (size() > other->size())
        {
            return 1;
        }
        // If all the keys and values compare equal, these are equal.
        Elements::const_iterator mine = _elements.begin();
        Elements::const_iterator theirs = other->_elements.begin();
        for (; mine != _elements.end() && theirs != other->_elements.end(); ++mine, ++theirs)
        {
            dint result = (*mine)->compare(**theirs);
            if (result) return result;
        }
        // These appear identical.
        return 0;
    }
    return Value::compare(value);
}

void ArrayValue::sum(Value const &value)
{
    ArrayValue const *array = dynamic_cast<ArrayValue const *>(&value);
    if (!array)
    {
        /// @throw ArithmeticError @a value was not an Array. ArrayValue can only be summed
        /// with another ArrayValue.
        throw ArithmeticError("ArrayValue::sum", "Array cannot be summed with value");
    }

    for (Elements::const_iterator i = array->_elements.begin(); i != array->_elements.end(); ++i)
    {
        _elements.push_back((*i)->duplicate());
    }
}

void ArrayValue::add(Value *value)
{
    _elements.push_back(value);
}

void ArrayValue::addMany(duint count, Value::Number value)
{
    while (count--)
    {
        add(new NumberValue(value));
    }
}

void ArrayValue::addMany(duint count, String const &value)
{
    while (count--)
    {
        add(new TextValue(value));
    }
}

void ArrayValue::add(String const &text)
{
    add(new TextValue(text));
}

Value const &ArrayValue::at(dint index) const
{
    return **indexToIterator(index);
}

ArrayValue::Elements::iterator ArrayValue::indexToIterator(dint index)
{
    if (index >= 0 && index < dint(size()))
    {
        return _elements.begin() + index;
    }
    else if (index < 0 && index > -dint(size()))
    {
        return _elements.begin() + size() + index;
    }
    else
    {
        /// @throw OutOfBoundsError @a index is out of bounds.
        throw OutOfBoundsError("ArrayValue::indexToIterator", "Index is out of bounds");
    }
}

ArrayValue::Elements::const_iterator ArrayValue::indexToIterator(dint index) const
{
    if (index >= 0 && index < dint(size()))
    {
        return _elements.begin() + index;
    }
    else if (index < 0 && index >= -dint(size()))
    {
        return _elements.begin() + size() + index;
    }
    else
    {
        /// @throw OutOfBoundsError @a index is out of bounds.
        throw OutOfBoundsError("ArrayValue::indexToIterator", "Index is out of bounds");
    }
}

void ArrayValue::insert(dint index, Value *value)
{
    if (index == dint(size()))
    {
        add(value);
    }
    else
    {
        _elements.insert(indexToIterator(index), value);
    }
}

void ArrayValue::replace(dint index, Value *value)
{
    Elements::iterator elem = indexToIterator(index);
    delete *elem;
    *elem = value;
}

void ArrayValue::remove(dint index)
{
    Elements::iterator elem = indexToIterator(index);
    delete *elem;
    _elements.erase(elem);
}

dint ArrayValue::indexOf(const Value &value) const
{
    for (int i = 0; i < _elements.size(); ++i)
    {
        if (!value.compare(*_elements[i]))
        {
            return i;
        }
    }
    return -1;
}

ArrayValue &ArrayValue::operator << (Value *value)
{
    add(value);
    return *this;
}

ArrayValue &ArrayValue::operator << (Value const &value)
{
    add(value.duplicate());
    return *this;
}

Value *ArrayValue::popLast()
{
    DENG2_ASSERT(size() > 0);
    return _elements.takeLast();
}

Value *ArrayValue::popFirst()
{
    DENG2_ASSERT(size() > 0);
    return _elements.takeFirst();
}

void ArrayValue::reverse()
{
    std::reverse(_elements.begin(), _elements.end());
}

StringList ArrayValue::toStringList() const
{
    StringList list;
    for (Value const *v : _elements)
    {
        list << v->asText();
    }
    return list;
}

void ArrayValue::clear()
{
    // Delete the values.
    for (Elements::iterator i = _elements.begin(); i != _elements.end(); ++i)
    {
        delete *i;
    }
    _elements.clear();
}

void ArrayValue::operator >> (Writer &to) const
{
    to << SerialId(ARRAY) << duint(_elements.size());
    for (Elements::const_iterator i = _elements.begin(); i != _elements.end(); ++i)
    {
        to << **i;
    }
}

void ArrayValue::operator << (Reader &from)
{
    SerialId id;
    from >> id;
    if (id != ARRAY)
    {
        /// @throw DeserializationError The identifier that species the type of the
        /// serialized value was invalid.
        throw DeserializationError("ArrayValue::operator <<", "Invalid ID");
    }
    // Number of elements.
    duint count = 0;
    from >> count;
    clear();
    while (count--)
    {
        add(Value::constructFrom(from));
    }
}

void ArrayValue::callElements(ArrayValue const &args)
{
    for (duint i = 0; i < size(); ++i)
    {
        Function const &func = at(i).as<FunctionValue>().function();
        Process(func.globals()).call(func, args);
    }
}

void ArrayValue::setElement(dint index, Number value)
{
    setElement(NumberValue(index), new NumberValue(value));
}

void ArrayValue::setElement(dint index, String const &value)
{
    setElement(NumberValue(index), new TextValue(value));
}

Value const &ArrayValue::element(dint index) const
{
    return element(NumberValue(index));
}

Value const &ArrayValue::operator [] (dint index) const
{
    return element(index);
}

String ArrayValue::asInfo() const
{
    StringList values;
    for (Value const *value : elements())
    {
        String text = value->asText();
        text.replace("\"", "''");
        values << String("\"%1\"").arg(text);
    }
    QString out;
    QTextStream os(&out);
    os.setCodec("UTF-8");
    os << "<"
       << String::join(values, ", ")
       << ">";
    return out;
}

Value::Text ArrayValue::typeId() const
{
    return "Array";
}

} // namespace de
