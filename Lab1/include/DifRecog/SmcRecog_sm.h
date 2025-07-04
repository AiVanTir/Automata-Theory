//
// ex: set ro:
// DO NOT EDIT.
// generated by smc (http://smc.sourceforge.net/)
// from file : SmcRecog.sm
//

#ifndef SMCRECOG_SM_H
#define SMCRECOG_SM_H


#define SMC_USES_IOSTREAMS

#include "DifRecog/statemap.h"

// Forward declarations.
class MAP1;
class MAP1_START;
class MAP1_TYPE;
class MAP1_SIZE;
class MAP1_SIZE_NUM;
class MAP1_SIZE_END;
class MAP1_EQUAL;
class MAP1_BRACE_OPEN;
class MAP1_FIRST_NUM;
class MAP1_NEXT_NUM;
class MAP1_CHECK;
class MAP1_ERROR;
class MAP1_Default;
class SmcRecogState;
class SmcRecogContext;
class SmcRecog;

class SmcRecogState :
    public statemap::State
{
public:

    SmcRecogState(const char * const name, const int stateId)
    : statemap::State(name, stateId)
    {};

    virtual void Entry(SmcRecogContext&) {};
    virtual void Exit(SmcRecogContext&) {};

    virtual void CLOSE_BRACE(SmcRecogContext& context);
    virtual void COMMA(SmcRecogContext& context);
    virtual void EQUAL_SIGN(SmcRecogContext& context);
    virtual void IDENT(SmcRecogContext& context);
    virtual void INT(SmcRecogContext& context);
    virtual void LBRACKET(SmcRecogContext& context);
    virtual void LONG(SmcRecogContext& context);
    virtual void NUMBER(SmcRecogContext& context);
    virtual void OPEN_BRACE(SmcRecogContext& context);
    virtual void RBRACKET(SmcRecogContext& context);
    virtual void SHORT(SmcRecogContext& context);

protected:

    virtual void Default(SmcRecogContext& context);
};

class MAP1
{
public:

    static MAP1_START START;
    static MAP1_TYPE TYPE;
    static MAP1_SIZE SIZE;
    static MAP1_SIZE_NUM SIZE_NUM;
    static MAP1_SIZE_END SIZE_END;
    static MAP1_EQUAL EQUAL;
    static MAP1_BRACE_OPEN BRACE_OPEN;
    static MAP1_FIRST_NUM FIRST_NUM;
    static MAP1_NEXT_NUM NEXT_NUM;
    static MAP1_CHECK CHECK;
    static MAP1_ERROR ERROR;
};

class MAP1_Default :
    public SmcRecogState
{
public:

    MAP1_Default(const char * const name, const int stateId)
    : SmcRecogState(name, stateId)
    {};

};

class MAP1_START :
    public MAP1_Default
{
public:
    MAP1_START(const char * const name, const int stateId)
    : MAP1_Default(name, stateId)
    {};

    virtual void INT(SmcRecogContext& context);
    virtual void LONG(SmcRecogContext& context);
    virtual void SHORT(SmcRecogContext& context);
};

class MAP1_TYPE :
    public MAP1_Default
{
public:
    MAP1_TYPE(const char * const name, const int stateId)
    : MAP1_Default(name, stateId)
    {};

    virtual void IDENT(SmcRecogContext& context);
};

class MAP1_SIZE :
    public MAP1_Default
{
public:
    MAP1_SIZE(const char * const name, const int stateId)
    : MAP1_Default(name, stateId)
    {};

    virtual void LBRACKET(SmcRecogContext& context);
};

class MAP1_SIZE_NUM :
    public MAP1_Default
{
public:
    MAP1_SIZE_NUM(const char * const name, const int stateId)
    : MAP1_Default(name, stateId)
    {};

    virtual void NUMBER(SmcRecogContext& context);
};

class MAP1_SIZE_END :
    public MAP1_Default
{
public:
    MAP1_SIZE_END(const char * const name, const int stateId)
    : MAP1_Default(name, stateId)
    {};

    virtual void RBRACKET(SmcRecogContext& context);
};

class MAP1_EQUAL :
    public MAP1_Default
{
public:
    MAP1_EQUAL(const char * const name, const int stateId)
    : MAP1_Default(name, stateId)
    {};

    virtual void EQUAL_SIGN(SmcRecogContext& context);
};

class MAP1_BRACE_OPEN :
    public MAP1_Default
{
public:
    MAP1_BRACE_OPEN(const char * const name, const int stateId)
    : MAP1_Default(name, stateId)
    {};

    virtual void OPEN_BRACE(SmcRecogContext& context);
};

class MAP1_FIRST_NUM :
    public MAP1_Default
{
public:
    MAP1_FIRST_NUM(const char * const name, const int stateId)
    : MAP1_Default(name, stateId)
    {};

    virtual void NUMBER(SmcRecogContext& context);
};

class MAP1_NEXT_NUM :
    public MAP1_Default
{
public:
    MAP1_NEXT_NUM(const char * const name, const int stateId)
    : MAP1_Default(name, stateId)
    {};

    virtual void CLOSE_BRACE(SmcRecogContext& context);
    virtual void COMMA(SmcRecogContext& context);
    virtual void NUMBER(SmcRecogContext& context);
};

class MAP1_CHECK :
    public MAP1_Default
{
public:
    MAP1_CHECK(const char * const name, const int stateId)
    : MAP1_Default(name, stateId)
    {};

};

class MAP1_ERROR :
    public MAP1_Default
{
public:
    MAP1_ERROR(const char * const name, const int stateId)
    : MAP1_Default(name, stateId)
    {};

};

class SmcRecogContext :
    public statemap::FSMContext
{
public:

    explicit SmcRecogContext(SmcRecog& owner)
    : FSMContext(MAP1::START),
      _owner(owner)
    {};

    SmcRecogContext(SmcRecog& owner, const statemap::State& state)
    : FSMContext(state),
      _owner(owner)
    {};

    virtual void enterStartState()
    {
        getState().Entry(*this);
        return;
    }

    inline SmcRecog& getOwner()
    {
        return (_owner);
    };

    inline SmcRecogState& getState()
    {
        if (_state == NULL)
        {
            throw statemap::StateUndefinedException();
        }

        return dynamic_cast<SmcRecogState&>(*_state);
    };

    inline void CLOSE_BRACE()
    {
        getState().CLOSE_BRACE(*this);
    };

    inline void COMMA()
    {
        getState().COMMA(*this);
    };

    inline void EQUAL_SIGN()
    {
        getState().EQUAL_SIGN(*this);
    };

    inline void IDENT()
    {
        getState().IDENT(*this);
    };

    inline void INT()
    {
        getState().INT(*this);
    };

    inline void LBRACKET()
    {
        getState().LBRACKET(*this);
    };

    inline void LONG()
    {
        getState().LONG(*this);
    };

    inline void NUMBER()
    {
        getState().NUMBER(*this);
    };

    inline void OPEN_BRACE()
    {
        getState().OPEN_BRACE(*this);
    };

    inline void RBRACKET()
    {
        getState().RBRACKET(*this);
    };

    inline void SHORT()
    {
        getState().SHORT(*this);
    };

private:
    SmcRecog& _owner;
};


#endif // SMCRECOG_SM_H

//
// Local variables:
//  buffer-read-only: t
// End:
//
