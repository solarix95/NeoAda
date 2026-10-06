# Foreword from the Author

## Why Another Scripting Language?

It is a fair question. There is already a large number of established programming languages, many of them supported by excellent tools, extensive libraries, and large communities.

In my own professional work, I have used several scripting languages in large industrial automation projects, including Lua, Python, and PHP. These languages are powerful and flexible. At the same time, while training new IT apprentices and working with technicians from fields such as electronics and automation, I repeatedly noticed that their apparent simplicity can be deceptive.

Many modern scripting languages make it very easy to get started. Yet for beginners, that simplicity often hides a number of traps: implicit type conversions, dynamic typing, subtle side effects, or language constructs whose behavior only becomes obvious with experience.

Professional software developers can usually handle these characteristics well. For people who are just beginning to program, or who use programming as a tool within another technical discipline, they can create unnecessary obstacles.

That experience led to the idea of NeoAda.

My goal was not simply to create one more scripting language. I wanted to design a language whose programs are well structured, understandable, and as predictable as possible — a language that makes the first steps easier without giving up the principles of disciplined software development.

## Inspiration from Ada 95

My enthusiasm for Ada 95 goes back many years. In particular, I was strongly influenced by ObjectAda and by John Barnes' classic book *Programming in Ada 95*.

What fascinated me was never only Ada's syntax. More important were the ideas behind it: explicit structure, strong typing, clear interfaces, and a style of programming in which readability and predictability are treated as important engineering properties.

Many of these principles come from a world in which software must not merely work, but must work reliably and in a way that can be understood and reviewed. The development of critical systems makes especially clear how important it is to avoid ambiguity, model state carefully, and make errors visible as early as possible.

These ideas are not limited to Ada or to safety-critical systems.

Many principles from the development of critical and high-integrity software can be transferred to almost any programming language. They influence how we design interfaces, manage state, handle failures, and structure programs.

In that sense, Ada 95 and the work of John Barnes have had a strong influence on my own programming style, regardless of which language I happen to be using.

NeoAda therefore does not merely borrow a few syntactic elements from Ada 95. It is an attempt to carry part of that philosophy into a compact and accessible scripting language.

## A Scripting Language for Learning and Practice

NeoAda was intended to be a scripting language from the beginning.

The connection with C++ is especially important to me. In industrial applications, the two can complement each other very well: a fast and controlled C++ core can provide the architecture, hardware-facing functions, and performance-critical behavior, while selected parts of the application logic can be moved into a well-structured scripting language.

This combination can produce applications that are both performant and flexible.

Logic can be adapted without necessarily changing and recompiling the entire C++ application. At the same time, the interface between the script and the application remains under the control of the C++ host.

I find this separation particularly valuable in technical and industrial applications. A technician or automation specialist should be able to understand and modify clearly defined sequence or decision logic without having to know every internal detail of a large C++ codebase.

A scripting language can become a comprehensible bridge between software engineering and technical application.

## Simplicity and Discipline Are Not Opposites

NeoAda grew out of two requirements that may at first seem very different.

The language should be simple enough to give beginners, students, and technically oriented users a clear introduction to programming. At the same time, it should be structured enough to remain useful in larger and more demanding applications.

I do not believe these goals are contradictory.

On the contrary, many properties that help beginners are the same properties that benefit professional software development.

Clear structure helps both learning and code review.

Explicit types help both understanding and error prevention.

Predictable behavior makes first programs easier to grasp and complex programs easier to analyze.

Understandable interfaces help students, technicians, and experienced developers alike.

A language should not have to choose between being easy to learn and supporting disciplined programming.

NeoAda is my attempt to bring those two goals together.

## About This Book

This book is therefore intended to be more than a description of NeoAda's syntax.

It should explain how programs are built, why certain language concepts exist, and how small, understandable building blocks can be combined into robust applications.

A beginner should be able to understand why a variable has a type or why a function should have a clearly defined responsibility.

At the same time, an experienced developer should be able to see the reasoning behind value semantics, error handling, and the controlled embedding of scripts into a C++ application.

The goal is not to copy Ada rules blindly. Nor is NeoAda intended to be a simplified clone of Ada 95.

The aim is to combine proven principles with the requirements of a modern embedded scripting language.

If NeoAda helps a student take the first steps into programming, allows a technician to express application logic clearly, and gives an experienced developer a practical way to extend a high-performance C++ core with flexible scripted behavior, then the language has achieved one of its most important goals.

And if someone starts with NeoAda and, through that accessible introduction, later discovers Ada 95 or a modern Ada environment, I would be especially pleased. Perhaps NeoAda can serve not only as a practical scripting language, but also as a bridge to the ideas and principles that have influenced my own programming for many years.
