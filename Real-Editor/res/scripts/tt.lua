local script1 = Real.Script.new()
entity1 = script1.entity
function script1:onCreate()
  print("created entity id: ", entity1:getId())
  entity1 = script1.entity
end
c1 = 100


function lerp(on , dest, speed)
  if(on == dest) then 
      return dest
  elseif on > dest then
      on = on - speed;
      return on;
  elseif on < dest then 
      on = on + speed;
      return on;
    end
  end

x = 0

function script1:onUpdate()
  --while true do
  --  print("yes")
  --end
  --scene = Real.AssetManager.getScene(Real.AssetManager.getUUID("../res/config/scene.Real"))
  --scene_camera_entity = scene.scene:getEntity("Camera");
  --print(Real.AssetManager.getUUID("../res/config/scene.Real"))
  c1 = c1 + 1
  t1 = script1:getTimestep():getTime()
  print(math.sin(t1))
  entity1:getSprite().color = Real.Vec4.new(1)--( math.sin(t1) * 100 ) - 0.6
  x = lerp(x, 300, 0.1)
  x = 50000
  li = entity1:insertForce();
  to = entity1:insertTorque();
  if(Real.Input.isKeyPressed(Real.Keyboard.A)) then
  print("W")
  li.magnitude = Real.Vec2.new(-x, 0)
  elseif(Real.Input.isKeyPressed(Real.Keyboard.D)) then
  print("S")
  li.magnitude = Real.Vec2.new(x, 0)
  elseif(Real.Input.isKeyPressed(Real.Keyboard.W)) then
  print("S")
  li.magnitude = Real.Vec2.new(0, x)
  elseif(Real.Input.isKeyPressed(Real.Keyboard.S)) then
  print("S")
  li.magnitude = Real.Vec2.new(0, -x)  
  elseif(Real.Input.isKeyPressed(Real.Keyboard.Q)) then
  to.magnitude = 40000
  elseif(Real.Input.isKeyPressed(Real.Keyboard.E)) then
  to.magnitude = -40000
  end

  --scene_camera_entity:insertTransform(entity1:getTransform())
end

function script1:onDestroy()
end
function GetObject()
  return script1
end
