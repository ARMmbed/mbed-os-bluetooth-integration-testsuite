# Extending the application

Extending the application means adding new command to existing modules and/or 
adding new command modules. Before diving in the extension of the application, 
it is important to understand how the application is structured and what it 
contains.

## Organization 

The application is divided in software modules which can be split into two 
categories: 

* Application specific code: This code is specific to ble-cliapp application.
* Reusable code: These component are not specific to ble-cliapp and it should be 
possible to reuse them in other projects. At some point they might move outside 
ble-cliapp in their own repository.

All these software modules are contained in the [`source`](source) folder.

### Reusable code

Four different modules can be considered as reusable: 

* `CLICommand`: C++ framework built on top of `mbed-cli-command`, it contains 
all the necessary primitives to create commands, group them into modules and 
register the modules in the system. 
* `EventQueue`: An implementation of a cross platform event queue. 
* `Serialization`: A very simple serialization framework which convert string 
values to C++ value and vice versa. It also contains a class which can format 
and stream JSON values. 
* `util`: Few utility classes.

### Application code:

The application specific code is located in the [`Commands`](source/Commands) 
folder. This module contains all the commands available in ble-cliapp. These 
commands are grouped into module where each of these module reflect a module of 
mbed BLE: 

- [`ble`](source/Commands/BLECommands.cpp)
- [`gap`](source/Commands/GapCommands.cpp)
- [`gattClient`](source/Commands/GattClientCommands.cpp)
- [`gattServer`](source/Commands/GattServerCommands.cpp)
- [`securityManager`](source/Commands/SecurityManagerCommands.cpp)

Serialization of mbed BLE specific type is located in the 
[Serialization](source/Commands/Serialization) subfolder 

Finaly, the file [`main.cpp`](source/main.cpp) glue the different part of the 
system together.

All code extending the application should be added in the Commands folder or 
main.cpp for the glue logic.


## Command creation

### Command concepts

A command is just an instance of the [Command structure](source/CLICommand/Command.h). 
The structure is collection of six function pointers. One of these function 
pointer is the command handler itself while the other five of those functions 
describe the command with metadatas.
These metadatas are used by the system to retrieve the command, help the user or 
validate arguments on the command line. 

* `const char* name()`: Should return the name of the Command. 
It is used by the system to retrieve and invoke the right command handler from 
the command line. It should **not** contain spaces.
* `const char* help()`: It is the help of the command.
* `ConstArray<CommandArgDescription> argsDescription()`: Description of the 
arguments expected by the command. The list of argument will be displayed when 
user asked for the help of the command.
* `ConstArray<CommandArgDescription> resultDescription()`: Description of the 
result returned by the command.
* `std::size_t maximumArgsRequired()`: Return the maximum number of arguments 
that the command can handle. This parameter is used to prefilter invalid command 
requests and  reduce the number of parameters that has to be checked by the 
handler. If the command line in input contains more arguments than the maximum 
number of arguments expected by the command, the command is not invoked and the 
system returns INVALID_PARAMETERS instead.  
* `void handler(const CommandArgs& args, const CommandResponsePtr& res)`: The 
actual command handler. It accept two arguments: 
  + args: Arguments pushed by the handler on the command line. The module name 
  and the command name are not included.
  + res: A CommandResponse object that the handler has to fullfill to answer the 
  command.



### Command declaration 

It is not advised to create command instances by hand. The prefered way is to 
define them with the helpers provided by 
[CLICommand/CommandHelper.h](source/CLICommand/CommandHelper.h). 

Goals of these helpers are: 
* Do not split Command declaration from command implementation to reduce the 
maintainance.
* Ease command construction. 
* Ensure correctness of the constness of the data structures.
* Reduce the boiler plate needed for the verification of the arguments from the 
command line.


#### Begining a command declaration 

To start a command declaration, use the macro `DECLARE_CMD(NAME)`. This will 
start a structure named `NAME` inheriting publicly from the structure 
`BaseCommand`. 
All elements of the command should go inside this structure. 

```c++
DECLARE_CMD(MyCommand) { 
    // elements of the command 
};
```

This is equivalent to 


```c++
struct MyCommand : public BaseCommand { 


};
```

#### Adding the command name 

It is easy to add the command name, either use the macro `CMD_NAME(QUOTED_NAME)` 
or create the static member function `const char* name()` inside the command 
being declared. The name of the command should **not** contains space.

```c++ 
DECLARE_CMD(MyCommand) { 
    CMD_NAME("my_command");
    // other parts of the command 
};
```

or 

```c++
struct MyCommand : public BaseCommand { 
    static const char* name() { 
        return "my_command"
    }
    // other parts of the command     
};
```

#### Adding help of the command 

It is not mandatory to add this function. If its not present, the system will 
use the implementation provided by the class `BaseCommand`. Once again the user 
have the choice to use a macros or declare the static member function by hand.


```c++ 
DECLARE_CMD(MyCommand) { 
    CMD_HELP("This is the help of my_command ....");
    // other parts of the command 
};
```

or 

```c++
struct MyCommand : public BaseCommand { 
    static const char* help() { 
        return "This is the help of my_command ...."
    }
    // other parts of the command     
};
```

#### Describing command parameters 

The declaration of command parameters is used for two things: 
* Complement the help of the command 
* Allow the system to automatically check that the number of arguments in input 
match the number of parameters expected by the command before invoking the 
actual command handler. 

Each parameters is described with three properties: 
* The type of the parametrer 
* The name of the parameter
* The description of the parameter 

Again it can be declared as a static function or using helper macros.

```c++ 
DECLARE_CMD(MyCommand) { 

    CMD_ARGS(
        CMD_ARG("<parameter type>", "<parameter name>", "<parameter description>"), 
        CMD_ARG("<parameter type>", "<parameter name>", "<parameter description>"),
        // other arguments  
    )

    // other parts of the command 
};
```

or 

```c++
struct MyCommand : public BaseCommand { 
    static ConstArray<CommandArgDescription> argsDescription() { 
        static const CommandArgDescription argsDescription[] = { 
            { "<parameter type>", "<parameter name>", "<parameter description>" },
            { "<parameter type>", "<parameter name>", "<parameter description>" },
            // other parameters ...
        }; 
        return ConstArray<CommandArgDescription>(argsDescription); 
    }
    // other parts of the command     
};
```

#### Describing command results 

Like the command parameters, this function provide a better help for the end 
user but unlike the command parameter the content of the command result does not 
have any effect at runtime on the command handler execution.


```c++ 
DECLARE_CMD(MyCommand) { 

    CMD_RESULTS(
        CMD_RESULT("<result type>", "<result name>", "<result description>"), 
        CMD_RESULT("<result type>", "<result name>", "<result description>"),
        // other results
    )

    // other parts of the command 
};
```

or 

```c++
struct MyCommand : public BaseCommand { 
    static ConstArray<CommandArgDescription> resultDescription() { 
        static const CommandArgDescription resultDescription[] = { 
            { "<result type>", "<result name>", "<result description>" },
            { "<result type>", "<result name>", "<result description>" },
            // other parameters ...
        }; 
        return ConstArray<CommandArgDescription>(resultDescription); 
    }
    // other parts of the command     
};
```

#### Command handler 

The command handler itself is achieved via the handler function:  

```c++
`void handler(const CommandArgs& args, const CommandResponsePtr& res)`
```

By default it accepts two parameters: 
* **args** which is just an array of strings matching containing the arguments 
of the command. 
* **res** the response of the command which is shared pointer to a 
`CommandResponse` object. The command should fullfill the content of the 
response by setting indicating if it succeed or fail and can send a JSON object 
as a result. 

The handler can be declared using an helper or directly as a static member 
function: 

```c++ 
DECLARE_CMD(MyCommand) { 

     CMD_HANDLER(const CommandArgs& args, CommandResponsePtr& response) { 
        // check arguments 
        // do something with them
        // fullfill the response 
     } 

    // other parts of the command 
};
```

or

```c++ 
DECLARE_CMD(MyCommand) { 

     static void handler(const CommandArgs& args, CommandResponsePtr& response) { 
        // check arguments 
        // do something with them
        // fullfill the response 
     } 

    // other parts of the command 
};
```

> **Note:** It is not required to check the number of arguments in input of the 
command, the runtime take care of it before invoking the handler.


It is also possible to let the runtime handle the deserialization of the 
arguments of the Command handler automatically. It is required to change the 
signature of the Command handler by putting the expected parameters before the 
command response parameter. The deserialization function for the arguments type 
should also be available.

```c++
void handler(A0 arg0, A1 arg1, ..., CommandResponsePtr& response);
```

> **Note:** Up to 10 arguments are supported.

> **Note:** References and values are supported in input.


#### Command instantiation 

As stated earlier commands are instances of the Command class. Declaring a 
comamnd the way as described above does **not** create an instance of the 
Command structure. To get access to the Command instance of the command class 
created, it is either possible to use an helper or generate it using the 
template class `CommandGenerator`. 

```c++
static const Command* my_command = CMD_INSTANCE(MyCommand);
```

or 

```c++
static const Command* my_command = &CommandGenerator<MyCommand>::command;
```


#### Complete example 

As a complete example consider an addition command which add two `int32_t` 
numbers: 

```C++
#include <CLICommand/CommandHelper.h>

DECLARE_CMD(AdditionCommand) {
    CMD_NAME("add")
    CMD_HELP("Add two number togethers and returns the result.")
    CMD_ARGS(
      CMD_ARG("int32_t", "lhs", "The left hand side of the operation"),
      CMD_ARG("int32_t", "rhs", "The right hand side of the operation")
    )
    CMD_RESULTS(
      CMD_RESULT("int64_t", "", "The result of lhs + rhs.")
    )

    CMD_HANDLER(int32_t lhs, int32_t rhs, CommandResponsePtr& response) { 
      int64_t result = lhs + rhs;
      response->success(result);
    }
};
```

Once attached to a command module and the command module registered in the 
system, the command line user will be able to list this command from its module,
get a detailed help about it and of course invoke the command: 

```
<module name> add <lhs> <rhs>
```

#### Few words about the Command design 

It would have been simpler to use an abstract class as the base class for the  
command rather than static polymorphism. Unfortunatelly using an abstract base 
class for Commands force the usage of virtual functions. As soon as a class 
defines virtual functions it cannot be instantiated at compilation time in read 
only memory (FLASH) because it is not a POD. Consequently objects referencing 
instances of a non POD class will also be created at runtime, in RAM, after the 
instantiation of the objects it reference. 

Worse, if these objects are global one (like a command or a list of commands) 
they will be created before main by the C++ runtime. To destroy these globals at 
exit, the C++ runtime has to book keep in RAM what has been instantiated and 
how to destroy it. Depending on the compiler used, this might involve dynamic 
memory allocation.

At the end, a global empty command will consume between 28 and 40 bytes of 
RAM. For an application like ble-cliapp and its 77 commands it means that 
between 2156 and 3080 bytes of RAM just for storing commands. This was an issue 
on the most constrained targets like the NRF51.

Another technical issue encountered by the use of an abstract Command class 
design was the increase of the code size. With abstract class design, a pair 
of destructor will be generated and added to the vtable of each command, 
additional destruction function will also be generated to cleanup global command 
objects. It was the cause of flash exhaustion on the NRF51 target.

Using C++11 (initialy it was the case!) instead of C++98 would have solve the 
all the memory issues induced by the use of abstract base class as well as 
allowing us to provide a clearer command declaration. 

Unfortunatelly until IAR supports C++11, the application will remains stuck with 
C++98.


#### Asynchronous command 

CLICommand provide the base class 
[AsyncProcedure](source/CLICommand/util/AsyncProcedure.h) to help the developer 
to write commands requiring asynchronous operations. 

The class manages automatically the lifetime of the state associated with the 
operation, the command response and the timeout if the operation didn't succeed 
in the expected time.

Consider the following procedure as an example: 

```c++
struct MyLongProcedure : public AsyncProcedure {
    // Construct the procedure. 
    // The state required by the procedure is passed at construction time. 
    // It is mandatory to pass a CommandResponse and a timeout as these values are 
    // needed by the base class AsyncProcedure.
    MyLongProcedure(A0 stateA, A1 stateB, ..., const CommandResponse& res, uint32_t timeout) :
        AsyncProcedure(res, procedureTimeout), _stateA(stateA), _stateB(stateB), ... {
    }

    // destructor defined if non RAII resources have to be released.
    virtual ~MyLongProcedure() { }

    // Procedure starting point
    // If the procedure launch was successful should returns true otherwise 
    // returns false
    virtual bool doStart() {
        // start the long operation, 
        // In this case, once the operation is done, whenResult member function 
        // will be called.
        // 
        return my_long_operation(_stateA, ..., this, &MyLongProcedure::whenResult)
    }

    // what to do when timeout occur 
    virtual void doWhenTimeout() { 
        // set response 
        // cleanup resources 
        // ...

        // DO NOT CALL terminate in this function.
    }

    //////////////////////////////
    // CODE SPECIFIC TO the class 
    //////////////////////////////

    // async handler, specific to MyLongProcedure
    void whenResult(bool success) { 
        // asynchronous operation done, fill the response (stored in the base
        // class)
        if (success) { 
            response->success();
        } else { 
            response->faillure("Oups!");
        }

        // terminate the procedure. This step **is** explicit and shouldn't be 
        // omitted.
        terminate();
    }

    A0 _stateA;
    A1 _stateB;
    // ... other state needed by the procedure
};
```

To launch the procedure, the helper function `startProcedure` has to be used 
from the command handler: 

```c++
startProcedure<MyLongProcedure>(a0, a1, ..., response, /* timeout */ 10000);
```


## Module creation 

A module is a structure declaring four static functions describing a cohesive 
group of commands. 

Lets start by creating a simple module: 

```c++
struct MyCommandModule {
    static const char* name() { 
        return "my_module";
    }

    static const char* info() { 
        return "infos about my module";
    }

    static const char* man() { 
        return "enter 'my_module list' to list the commands available in this 
        module and 'my_module help <command name>' to get the help of a command.";
    }

    // not implemented here
    static ConstArray<const Command*> commands();
};
```

* `const char* name()`: Return the name used to access to the module. It 
should not contain space. 
Users will use this value to gain access to the commands of the module on the CLI. 
For instance, if this function returns `my_module`, users will access commands 
by invoking `my_module <command_name> <args...>`.
* `const char* info()`: Short readable description of the module.
* `const char* man()`: Manual of the module. It can just instruct the user how 
to use the builtin `list` and `help`.
* `ConstArray<const Command*> commands()`: Return an array containing all the 
commands of the module.


Like for Commands declaration, it is possible to use an helper to define the 
`commands` function: 

```c++
DECLARE_SUITE_COMMANDS(MyCommandModule, 
    <command instances...>
)
```

If the commands have been declared with the `DECLARE_CMD` helper then it is 
possible to use the helper `CMD_INSTANCE` to get the instance of the command.

For instance, with the addition command defined earlier: 

```c++
DECLARE_SUITE_COMMANDS(MyCommandModule, 
    CMD_INSTANCE(AdditionCommand),
    <other commands instances>
)
```

## Register command suite 

To register a new command suite in the system, the user have to call the 
template function `registerCommandSuite` with the type of the command suite as 
template argument. This call must be made in the function 
`initialize_app_commands` defined in the file `main.cpp`.

```c++
void initialize_app_commands(void) {
    // existing code ....

    // register new command module here:
    registerCommandSuite<MyCommandModule>();
}
```

## Serialization 

The serialization framework define a simple protocol to serialize c++ values 
into JSON and deserialize string into c++ values. 

Since the framework was built around mbed-client-app needs and limitations it 
is assymetric: 
* Deserialization of arguments in input convert string tokens into c++ value. It 
is **not** structured and it is the job of the command handler to reconstruct 
the aggregates (if any). 
* Serialization of the command response is structured and made in JSON. 

Therefore, it is common that aggregates type only implement the serialization 
to JSON.

### String deserialization

One functions is the pilar of the string deserialization process. For a type T, 
this function would be defined the following way: 

```c++
bool fromString(const char* input, T& output);
```

This function takes a string in input and fill the output variable if it is 
possible to convert the input in a T. The returned value of the operation is the 
status of the operation: `true` if it has succeed and `false` otherwise.

Users wishing to generate automatically the deserialization part of their 
command handler should provide an overload of this function for any type in 
input of the command handler. 

### Enum deserialization/serialization

Serialization and deserialization of enumerations can be achieved in a 
declarative fashion rather than an imperative one. User code should describe 
what the enumeration is and what is the mapping between string values and 
enumeration values rather than implementing functions directly. 

Users just have to provide a complete specialization for the enum type of the 
class template SerializerDescription (see 
[here](source.Serialization/Serializer.h)). The specialization requires two 
**static** member functions: 

* `const ConstArray<ValueToStringMapping<T> > mapping()`: This function should 
return a map between string values and enum values. 
* `const char* errorMessage()`: Message in case of error during the 
deserialization process.

As an example consider the serialization of the following enumeration: 

```c++
enum Gender { 
  MALE, 
  FEMALE
};
```

The description can be achieved with the following specialization:

```c++
template<>
struct SerializerDescription<Gender> {
    typedef Gender type;

    static const ConstArray<ValueToStringMapping<type> > mapping() {
        static const ValueToStringMapping<type> map[] = {
            { MALE, "male" },
            { FEMALE, "female" }
        };

        return makeConstArray(map);
    }

    static const char* errorMessage() {
        return "unknown Gender";
    }
};
```

No JSON serialization function are automatically generated, instead a function 
converting the enum to a string is generated (`const char* toStringT()`). 

User code should either declare the operator `<<` for the type T with the 
JSONOutput stream or stream value using the `toString` function.

```c++
using namespace serialization;

JSONOutputStream os;
Gender g = MALE;

// will add the string "male" into the stream
os << toString(g);

// overload of operator << for Gender
JSONOutputStream& operator<<(JSONOutputStream& os, Gender gender) { 
  return os << toString(gender);
}

os << g;
```


### JSON serialization

Serialization to JSON is achieved with a 
[JSONOutputStream](source/Serialization/JSONOutputStream.h). Providing an 
overload of the operator `<<` for this stream is enough to achieve the 
serialization process: 

```c++
namespace ser = serialization;
ser::JSONOutputStream& operator<<(ser::JSONOutputStream& os, const T& v);
```

All the tools necessary to declare and structure JSON are bundled with the 
JSONOutputStream: 


#### Number serialization 

Serialization of numbers is done using the operator `<<`.

```c++ 
using namespace serialization;

JSONOutputStream os;

uint16_t value = 42;

// will ad the number 42 in the JSON stream
os << value;
```


#### String serialization 

Like numbers, string serialization is implemented by the operator `<<`.

```c++ 
using namespace serialization;

JSONOutputStream os;
const char* value = "Hello Word!";

// will add the string "Hello Word!" in the JSON stream
os << value;
```

It is important to note that the string added in the JSON stream is **not** 
escaped.


#### Boolean serialization 

Like others serialization of boolean is implemented by the operator `<<`.

```c++ 
using namespace serialization;

JSONOutputStream os;
bool value = true;

// will add the boolean true in the JSON stream
os << value;
```


#### Null serialization 

Null value can be added to a JSON stream by pushing the function `nil`.

```c++ 
using namespace serialization;

JSONOutputStream os;
// will add the nill value in the JSON stream
os << nil;
```


#### Object declaration 

To create a JSON object insert the function `startObject` into the stream. 
To create properties, insert keys using the function `key` followed by the value 
associated with the key. Finally to end the object insert the function 
`endObject` in the stream.


```c++ 
using namespace serialization;

struct Pixel { 
    uint8_t red; 
    uint8_t green; 
    uint8_t blue; 
};

// JSON serialization function of a Pixel 
JSONOutputStream& operator<<(JSONOutputStream& os, const Pixel& p) { 
    return os << startObject <<
        key("red") << p.red <<
        key("green") << p.green <<
        key("blue") << p.blue <<
    endObject;
}


// usage 
JSONOutputStream os;
Pixel p = { 0, 0. 255 };

os << p;
```

#### Array declaration 

Arrays can be created by inserting the function `startArray` and closed by 
inserting the function `endArray`. 

As a simple example consider JSON serialization of this POD which mix recursion,
object and arrays: 

```c++ 
struct Person { 
  const char* name; 
  uint8_t age;
  struct Person* childrens; 
  uint32_t childrens_count; 
};
```

The POD itself can be converted to a JSON object where each property map the 
name and the value of a POD field. The field `childrens` can be converted into 
a JSON array of Person. 

```c++
using namespace serialization;

JSONOutputStream& operator<<(JSONOutputStream& os, const Person& p);

JSONOutputStream& operator<<(JSONOutputStream& os, const Person& p) { 
  // Person is an aggregate, declare it inside an object.
  os << startObject <<
    // each field of the POD match a property
    key("name") << p.name <<
    key("age") << p.age <<
    key("childrens_count") << p.childrens_count <<
    // childrens is a list of Person, store them into a JSON array.
    key("childrens") << startArray;  
    for (uint32_t i = 0; i < p.childrens_count; ++i) { 
      os << childrens[i];
    }
    // end the array of childrens 
    os << endArray <<
  // end the person object
  endobject;
}

```
