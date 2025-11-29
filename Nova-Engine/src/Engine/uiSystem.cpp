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
	entt::entity hoveringButtonId = entt::null;
	glm::vec2 screenSpacePosition = window.getUISpacePos();

	// determine which button is being hovered..
	for (auto&& [entity, transform, entityData, button] : registry.view<Transform, EntityData, Button>().each()) {
		if (!entityData.isActive || !engine.ecs.isComponentActive<Button>(entity) || !button.isInteractable) {
			continue;
		}

		if (Math::isPointInRect(screenSpacePosition, transform.position + button.offset, transform.scale + button.padding) && hoveringButtonId == entt::null) {
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

			// determine if the pressed button was released.. this check is done outside
			if (button.state == Button::State::Pressed && leftMouseReleased) {
				button.timeElapsed = 0.f;
				button.state = Button::State::Hovered;

				executeButtonCallback(button, button.onClickReleasedFunction);
			}
		}
		else {
			button.state = Button::State::Normal;
			button.finalColor = button.normalColor;
		}

		// update the color lerp of all buttons.. (hovering & pressed)
		if (button.state != Button::State::Normal) {
			float lerpFactor = std::clamp(button.timeElapsed / button.fadeDuration, 0.f, 1.f);

			ColorA startColor;
			ColorA destinationColor;
			if (button.state == Button::State::Hovered) {
				startColor = button.normalColor;
				destinationColor = button.highlightedColor;
			}
			else if (button.state == Button::State::Pressed) {
				startColor = button.highlightedColor;
				destinationColor = button.pressedColor;
			}

			button.finalColor = glm::mix(glm::vec4{ startColor }, glm::vec4{ destinationColor }, lerpFactor) * std::lerp(1.f, button.colorMultiplier, lerpFactor);

			if (button.timeElapsed < button.fadeDuration) {
				button.timeElapsed += dt;
			}
		}
	}

	toSelect = false;
	leftMouseReleased = false;
}

void UISystem::executeButtonCallback(Button const& button, std::string const& functionName) {
	if (functionName.size() && button.reference.entity != entt::null && button.reference.script != INVALID_RESOURCE_ID) {
		engine.scriptingAPIManager.executeFunction(button.reference.entity, button.reference.script, functionName);
	}
}
