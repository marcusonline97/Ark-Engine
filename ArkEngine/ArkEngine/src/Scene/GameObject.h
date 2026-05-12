#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Engine
{
	class GameObject
	{

	public:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------

		//-------------------------------------------
		// Functions
		//-------------------------------------------
		virtual ~GameObject() = default;
		virtual void Update(float deltaTime);
		const std::string& GetName() const;
		void SetName(const std::string& name);
		GameObject* GetParent();
		bool IsAlive() const;
		void MarkForDestroy();

	protected:

		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------

		//-------------------------------------------
		// Functions
		//-------------------------------------------
		GameObject() = default;

	private:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------
		std::string m_name;
		GameObject* m_parent = nullptr;
		std::vector<std::unique_ptr<GameObject>> m_children;

		bool m_isAlive = true;

		friend class Scene;

		//-------------------------------------------
		// Functions
		//-------------------------------------------

	};
}