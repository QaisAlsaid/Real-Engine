#ifndef SCENE_HIERARCHY_H
#define SCENE_HIERARCHY_H

#include "EditorEvents/EditorEvents.h"
#include "EditorEvents/SceneEvents.h"
#include "EditorEvents/MousePickEvents.h"
#include <Real-Engine/Real-Engine.h>


namespace Real
{
  class SceneHierarchy
  {
  public:
    SceneHierarchy();
    SceneHierarchy(const ARef<Scene>& context);

    void setContext(const ARef<Scene>& context);
    inline ARef<Scene> getContext() { return m_context; }
    void onGuiUpdate();
    inline Entity getCurrentSelected() { return m_current; }
    void setCurrentSelected(Entity e);
    void clearSelection() { setCurrentSelected({});}
    
    void onEditorUpdate(EditorEvent& e);
    bool onSceneChangedCall(SceneChangedEvent& e);
    bool onSceneSetCall(SceneSetEvent& e);
    bool onMousePickedChangedCalll(MousePickedChangedEvent& e);
  private:
    void drawEntityNode(Entity& entity);
  private:
    ARef<Scene> m_context;//TODO:weakRef
    Entity m_current;
    uint32_t m_sub_id;
  };
}
#endif //SCENE_HIERARCHY_H
