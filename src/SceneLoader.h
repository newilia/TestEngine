#pragma once

class Scene;

class SceneLoader {
public:
	SceneLoader(Scene* scene);
	void loadScene();
private:
	Scene* mScene;
};