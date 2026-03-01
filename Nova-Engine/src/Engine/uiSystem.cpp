#include "uiSystem.h"

#include "Engine/engine.h"
#include "InputManager/inputManager.h"
#include "Engine/window.h"

#include "nova_math.h"

constexpr ColorA whiteColor { 1.f, 1.f, 1.f, 1.f };

UISystem::UISystem(Engine& engine) :
	engine				{ engine },
	registry			{ engine.ecs.registry },
	inputManager		{ engine.inputManager },
	clickedButtonId		{ entt::null },
	window				{ engine.window },
	toSelect			{ false },
	leftMouseReleased	{ false }
{
	inputManager.subscribe<ToSelectGameObject>(
		[&](ToSelectGameObject) {
			toSelect = true;
		},
		[&](ToSelectGameObject) {
			leftMouseReleased = true;
		}
	);

	//Hear construct and destroy functions ------------
	registry.on_construct<Button>().connect<&UISystem::onButtonCreation>(*this);
}

void UISystem::update(float dt) {
	if (engine.isInSimulationMode()) {
		updateSimulation(dt);
	}
	else {
		updateNonSimulation();
	}
}

void UISystem::updateNonSimulation() {
	for (auto&& [entity, transform, entityData, button] : registry.view<Transform, EntityData, Button>().each()) {
		button.state = Button::State::Normal;
		button.finalColor = button.normalColor;
	}
}

void UISystem::updateSimulation(float dt) {
	glm::vec2 screenSpacePosition = window.getUISpacePos();

	// determine which button is being hovered..
	for (auto&& [entity, transform, entityData, button] : registry.view<Transform, EntityData, Button>().each()) {
		if (!entityData.isActive || !engine.ecs.isComponentActive<Button>(entity)) {
			continue;
		}

		bool isMouseOnButton = Math::isPointInRect(screenSpacePosition, transform.position + button.offset, transform.scale + button.padding);

		if (isMouseOnButton) {
			if (button.isInteractable) {
				// determine if the hover was first frame..
				if (button.state == Button::State::Normal) {
					button.timeElapsed = 0.f;
					button.state = Button::State::Hovered;

					executeButtonCallback(button, button.onHoverFunction);
				}

				// determine if the hovered button is clicked..
				if (button.state == Button::State::Hovered && toSelect) {
					button.timeElapsed = 0.f;
					button.state = Button::State::Pressed;

					executeButtonCallback(button, button.onPressFunction);
				}
			}
		}
		// transition back to normal when not hovered over.. 
		else if (button.state == Button::State::Hovered) {
			button.state = Button::State::Normal;
			button.finalColor = button.normalColor;

			executeButtonCallback(button, button.onHoverLeaveFunction);
		}
		
		// determine if the pressed button was released.. this check is done outside
		if (button.state == Button::State::Pressed && leftMouseReleased) {
			button.timeElapsed = 0.f;
			button.state = isMouseOnButton ? Button::State::Hovered : Button::State::Normal;

			executeButtonCallback(button, button.onClickReleasedFunction);

			if (button.state == Button::State::Normal) {
				executeButtonCallback(button, button.onHoverLeaveFunction);
				button.finalColor = button.normalColor;
			}
		}

		// update the color lerp of all buttons.. (hovering & pressed)
		updateButtonColor(button);

		if (button.timeElapsed < button.fadeDuration) {
			button.timeElapsed += dt;
		}
	}

	toSelect = false;
	leftMouseReleased = false;
}

void UISystem::updateButtonColor(Button& button) {
	switch (button.state) {
	case Button::State::Normal:
		button.finalColor = button.normalColor;
		break;
	case Button::State::Disabled:
		button.finalColor = button.disabledColor;
		break;
	default:
		float lerpFactor = std::clamp(button.timeElapsed / button.fadeDuration, 0.f, 1.f);

		glm::vec4 startColor;
		glm::vec4 destinationColor;

		if (button.state == Button::State::Hovered) {
			startColor = button.normalColor;
			destinationColor = glm::vec4{ button.highlightedColor } * button.colorMultiplier;
		}
		else if (button.state == Button::State::Pressed) {
			startColor = glm::vec4{ button.highlightedColor } * button.colorMultiplier;
			destinationColor = glm::vec4{ button.pressedColor } * button.colorMultiplier;
		}

		button.finalColor = glm::mix(startColor, destinationColor, lerpFactor);
	}
}

void UISystem::executeButtonCallback(Button const& button, std::string const& functionName) {
	if (functionName.size() && button.reference.entity != entt::null && button.reference.script != INVALID_RESOURCE_ID) {
		engine.scriptingAPIManager.executeFunction(button.reference.entity, button.reference.script, functionName);
	}
}

void UISystem::onButtonCreation(entt::registry&, entt::entity entityId) {
	Button& button = registry.get<Button>(entityId);
	button.state = Button::State::Normal;
	button.finalColor = button.normalColor;
}
