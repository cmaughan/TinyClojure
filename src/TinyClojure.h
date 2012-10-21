// Copyright (C) 2012 Duncan Steele
// http://slidetocode.com
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//  TinyClojure.h
//  TinyClojure
//
//  Created by Duncan Steele on 20/10/2012.
//

#ifndef __TinyClojure__TinyClojure__
#define __TinyClojure__TinyClojure__

#include <iostream>
#include <string>
#include <set>
#include <map>
#include <vector>

namespace tinyclojure {
    /**
     * A class/interface for the interpreters IO
     *
     * Subclassing this allows for easier
     */
    class IOProxy {
    public:
        void writeOut(std::string stringout) {
            std::cout << stringout;
        }
        
        void writeErr(std::string stringout) {
            std::cerr << stringout;
        }
    protected:
        
    };

    
    /**
     * class to represent Clojure objects
     */
    class Object {
    public:
        typedef enum {
            kObjectTypeString,
            kObjectTypeSymbol,
            kObjectTypeCons,
            kObjectTypeNil,
            kObjectTypeInteger,
        } ObjectType;
        
        /// construct either a symbol (if symbol=true) or a string object otherwise
        Object(std::string stringValue, bool symbol=false);
        
        /// construct a nil object
        Object();
        
        /// construct an integer object
        Object(int intValue);
        
        /// construct a pair
        Object(Object *left, Object *right);
                
        ~Object();
        
        /// this object's type
        ObjectType type();
        
        /// return a reference to this object as a string value
        std::string& stringValue();
        
        /// return a reference to this object as an integer value
        int& integerValue();
        
        /// build a string representation of the object
        std::string stringRepresentation();
        
    protected:
        ObjectType _type;
        union {
            std::string* stringValue;
            struct {
                Object *left, *right;
            } consValue;
            int integerValue;
        } pointer;
    };
    
    /**
     * An object to represent the parser state
     *
     * construct it with the string to be parsed
     * parts of the parser actually live in here
     */
    class ParserState {
    public:
        ParserState(std::string& stringin) : parserString(stringin) {
            position = 0;
        }
        
        std::string& parserString;
        int position;
                
        /**
         * skip newlines and whitespace.  A convenience wrapper for skipCharactersInString
         */
        int skipNewLinesAndWhitespace() {
            return skipCharactersInString(" \n\r\t");
        }
        
        /**
         * a clojure separator set
         */
        int skipSeparators() {
            return skipCharactersInString(" \t,\n\r");
        }
        
        /**
         * the character currently pointed to by the parser state
         */
        char currentChar() {
            return parserString[position];
        }
        
        /**
         * Safely peek ahead one character, returning 0 if this would run off the end of the string
         */
        char peekChar();
        
        /**
         * Safely peek ahead two characters, returning 0 if this would run off the end of the string
         */
        char peekPeekChar();
        
        /**
         * return true when the state position is within the bounds of the string
         */
        bool charactersLeft() {
            return position < parserString.length();
        }
        
        /**
         * advance the parser through any characters in the passed string
         *
         * return the number of characters skipped
         */
        int skipCharactersInString(std::string skipSet);
    };
    
    /**
     * An object to represent the interpreter's state (probably more accurately, a single scope)
     */
    class InterpreterScope {
    public:
        
    protected:
        
    };
    
    /**
     * Wrap a parser error to make it easy to throw an exception
     */
    class ParserError {
    public:
        ParserError(ParserState &state, std::string errorMessage) : message(errorMessage), position(state.position) {
            
        }
        
        std::string message;
        int position;
    };
    
    /**
     * a very simple garbage collector for TinyClojure objects
     *
     * TODO nothing is really implemented yet
     *
     * the ExportedObject is essentially a C++ reference counting mechanism to keep track of "root objects" ie objects being used in the real world
     * when a garbage collection happens connectivity to these objects is the criteria for garbage collecting an object or not
     */
    class GarbageCollector {
    public:
        GarbageCollector();
        ~GarbageCollector();
        
        /**
         * register an object with the garbage collector
         */
        Object* registerObject(Object* object);
        
        
        /**
         * increment the "root object" reference count for this Object
         */
        Object* retainRootObject(Object *object);
        
        /**
         * decrement the "root object" reference count for this Object
         */
        Object* releaseRootObject(Object *object);
        
        /**
         * call this to start a garbage collection run
         */
        void collectGarbage();
        
    protected:
        std::set<Object*> _objects;
        std::map<Object*, int> _rootObjects;
    };
    
    /**
     * a wrapper for Object* when exporting any object
     *
     * this helps the garbage collector keep track of root objects still exist
     * DO NOT USE (read TODOs)
     *
     * TODO all objects exported from the TinyClojure should be exported via this
     * TODO right the garbage collector this references will be destroyed before this object is destroyed, ideally the garbage collector should be reference counted
     */
    class ExportedObject {
    public:
        ExportedObject(GarbageCollector *gc, Object* ptr) : _gc(gc), _object(ptr) {
            _gc->retainRootObject(_object);
        }
        
        ~ExportedObject() {
            _gc->releaseRootObject(_object);
        }
        
        Object& operator* () {
            return *_object;
        }
        
        Object* operator-> () {
            return _object;
        }
        
    protected:
        GarbageCollector *_gc;
        Object *_object;
    };
    
    class TinyClojure {
    public:
        /**
         * parse the passed string, returning the parsed objects, or NULL on error
         *
         * this will parse the data into a tree of S Expressions.  This throws an exception if there is a parser error.
         *
         * @return either an object created by parsing the input, or NULL if nothing was found
         */
        Object* parse(std::string stringin);
        
        /**
         * evaluate the code passed above
         */
        Object* eval(Object* object);
        
        /**
         * default constructor
         */
        TinyClojure();
        
        /**
         * destructor
         */
        ~TinyClojure();
        
    protected:
        /// the IO proxy for this interpreter
        IOProxy *_ioProxy;
        
        /**
         * create a list from a std::vector
         *
         * this is here, not in the Object constructor because it needs access to the garbage collector
         */
        Object* listObject(std::vector<Object*> list);
        
        GarbageCollector *_gc;
        
        /// the internal recursive parser function, see parse for documentation
        Object* recursiveParse(ParserState& parseState);
        
        std::string _newlineSet, _excludeSet, _numberSet;
    };
}

#endif /* defined(__TinyClojure__TinyClojure__) */