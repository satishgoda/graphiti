#pragma once

#include <raindance/Core/Interface/Documents/Shell.hh>
#include <raindance/Core/Interface/Documents/TextArea.hh>

#include <graphiti/Entities/MVC.hh>
#include <graphiti/Core/Console.hh>

class GraphitiStdout : public TextArea
{
public:
	GraphitiStdout(Document::Node* parent = NULL)
	: TextArea(parent)
	{
	}

    void onScreenshot(bool enter) override
    {
    	style().Visible = !enter;
    }
};

class GraphitiShell : public Shell, public EntityListener
{
public:
	GraphitiShell(Document::Node* parent = NULL)
	: Shell(parent), EntityListener()
	{
		m_Console = NULL;
	}

    virtual ~GraphitiShell()
    {
    }

    void onScreenshot(bool enter) override
    {
    	style().Visible = !enter;
    }

	void bind(GraphitiConsole* console)
	{
		m_Console = console;
		m_Console->listeners().push_back(this);
	}

	virtual void notify(IMessage* message)
	{
		if (message->type() != IMessage::CONSOLE)
			return;

		auto msg = static_cast<ConsoleMessage*>(message);
		this->print(msg->Message);
	}

	void execute(const std::string& command) override
	{
		if (m_Console == NULL)
			print(command);
		else
			m_Console->execute(command);
	}

private:
	GraphitiConsole* m_Console;
};
