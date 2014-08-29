#pragma once

#include <raindance/Core/Context.hh>
#include <raindance/Core/Controller.hh>
#include <raindance/Core/Variables.hh>

// Forward declarations

class Entity;

// ----- Entity Base -----

class EntityBase
{
public:
    inline Variables& attributes() { return m_Attributes; }

protected:
    Variables m_Attributes;
};

// ----- Entity Context -----

class EntityContext : public Context
{
public:
};

// ----- Entity Model -----

class EntityModel : public EntityBase
{
public:
    virtual ~EntityModel() = 0;
};

EntityModel::~EntityModel() {}


// ----- Entity View -----

class EntityView : public EntityBase
{
public:
    virtual ~EntityView() = 0;
    virtual const char* name() const = 0;
    virtual void draw() = 0;
    virtual void idle() = 0;
    virtual bool bind(Entity* entity) = 0;

    virtual IVariable* getAttribute(const std::string& name) = 0;
};

EntityView::~EntityView() {}

// ----- Entity Controller -----

class EntityController : public EntityBase, public Controller
{
public:
    virtual ~EntityController() = 0;
    virtual void draw() = 0;
    virtual void idle() = 0;
};

EntityController::~EntityController() {}

// ----- Entity Listener -----

class EntityListener : public EntityBase, public IListener
{
public:
    virtual ~EntityListener() = 0;
    virtual void onSetAttribute(const std::string& name, VariableType type, const std::string& value) = 0;

};
EntityListener::~EntityListener() {}

// ----- Entity -----

class Entity : public EntityBase
{
public:
    typedef unsigned long ID;

    enum Type
    {
        GRAPH,
        TIME_SERIES
    };

    Entity(Type type)
    {
        m_Type = type;
    }

    virtual ~Entity()
    {
        for (auto c : controllers())
            delete c;

        for (auto v : views())
            delete v;

        // TODO : Why does it crash ?
        // for (auto l : listeners())
        //    delete l;
    }

    virtual void send(const Variables& input, Variables& output) = 0;

    void setAttribute(const std::string& name, const std::string& type, const std::string& value)
    {
        VariableType vtype;

        if (type == "float")
            vtype = RD_FLOAT;
        else if (type == "string")
            vtype = RD_STRING;
        else if (type == "int")
            vtype = RD_INT;
        else if (type == "bool")
            vtype = RD_BOOLEAN;
        else if (type == "vec2")
            vtype = RD_VEC2;
        else if (type == "vec3")
            vtype = RD_VEC3;
        else if (type == "vec4")
            vtype = RD_VEC4;
        else
        {
            std::cout << "Unknown attribute type \"" << type << "\" !" << std::endl;
            return;
        }

        unsigned long pos = name.find(":");
        std::string category = name.substr (0, pos);
        std::string sname = name;

        // TODO : Remove 'raindance' attribute namespace whenever possible.
        if (category == "raindance" || category == "graphiti" || category == "og")
        {
            sname = sname.substr(pos + 1);
        }
        else
        {
            model()->attributes().set(sname, vtype, value);
        }

        for (auto l : listeners())
            l->onSetAttribute(sname, vtype, value);
    }

    IVariable* getAttribute(const std::string& name)
    {
        unsigned long pos1 = name.find(":");
        std::string category = name.substr (0, pos1);

        // TODO : Remove 'raindance' attribute namespace whenever possible.
        if (category == "raindance" || category == "graphiti" || category == "og")
        {
            std::string rest = name.substr(pos1 + 1);
            unsigned long pos2 = rest.find(":");
            std::string view = rest.substr(0, pos2);
            rest = rest.substr(pos2 + 1);

            for (auto v : views())
            {
                if (view == std::string(v->name()))
                    return v->getAttribute(rest);
            }
            return NULL;
        }
        else
        {
            IVariable* attribute = model()->attributes().get(name);
            if (attribute)
                return attribute->duplicate();
            else
                return NULL;
        }
    }

    virtual EntityContext* context() = 0;
    virtual EntityModel* model() = 0;
    inline std::list<EntityView*>& views() { return m_Views; }
    inline std::list<EntityController*>& controllers() { return m_Controllers; }
    inline std::vector<EntityListener*>& listeners() { return m_Listeners; }

    inline Type type() { return m_Type; }

private:
    Type m_Type;

    std::list<EntityView*> m_Views;
    std::list<EntityController*> m_Controllers;
    std::vector<EntityListener*> m_Listeners;
};

#include "Graph/GraphEntity.hh"

class EntityManager
{
public:
    EntityManager()
    {
        m_NextEntityID = 0;
        m_ActiveEntity = NULL;
    }

    virtual ~EntityManager()
    {
        for (auto e : m_Entities)
            delete e.second;
    }

    Entity::ID create(const char* type)
    {
        std::string stype(type);

        Entity::ID id = m_NextEntityID;

        if (stype == "graph")
            m_Entities[id] = new GraphEntity();
        else
        {
            LOG("[ENTITY] '%s' : Unknown entity type!\n", type);
            throw;
        }

        m_NextEntityID++;
        return id;
    }

    void destroy(Entity::ID id)
    {
        Entity* e = m_Entities[id];
        if (m_ActiveEntity == e)
            m_ActiveEntity = NULL;
        m_Entities.erase(id);
        delete e;
    }

    void bind(Entity::ID id)
    {
        m_ActiveEntity = entity(id);
    }

    // Accessors
    inline Entity* entity(Entity::ID id) { return m_Entities[id]; }
    inline Entity* active() { return m_ActiveEntity; }
    inline std::unordered_map<Entity::ID, Entity*>& entities() { return m_Entities; }

private:
    Entity::ID m_NextEntityID;
    Entity* m_ActiveEntity;
    std::unordered_map<Entity::ID, Entity*> m_Entities;
};
