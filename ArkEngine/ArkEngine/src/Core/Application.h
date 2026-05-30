#pragma once

namespace Engine
{
	class Application
	{
	public:
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------


		//-------------------------------------------
		// Functions
		//-------------------------------------------
		virtual ~Application() = default;
		virtual void RegisterTypes();
		virtual bool Init() = 0;
		//deltaTime in seconds
		virtual void Update(float deltaTime) = 0;
		virtual void RenderUI();

		virtual void Destroy() = 0;

		void SetNeedsToBeClosed(bool value);

		bool NeedsToBeClosed() const;

	private: 
		//-------------------------------------------
		// Properties & Variables
		//-------------------------------------------
		bool m_needsToBeClosed = false;
	
		//-------------------------------------------
		// Functions
		//-------------------------------------------

	};
}