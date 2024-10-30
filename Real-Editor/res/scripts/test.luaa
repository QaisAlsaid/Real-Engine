local script = Real.Script.new()
local sprite;
local trans;
local asset_manager;
local scene;
local scene_camera_entity;
local body;
local karen = Real.Vec4.new(1);
local col = Real.Vec4.new(255);
local move;
local circle;

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

print("made the create")
function script:onCreate()
  script:export("Real", karen)
  script:export("Color", col, true)


  asset_manager = Real.App.getAssetManager();
  print(asset_manager)
  scene = asset_manager:getScene("scene")
  print("making entity")
  --e = Real.Entity.new(3, scene);
  --print("entity id: ", e:getId());
  scene_camera_entity = scene:getEntity("Camera");
  print("onCreate Called")
  print("id", script.entity:getId());
  print("getting sprite")
  --sprite = Real.Scene.Components.Sprite.new();
  --sprite = script.entity:addSprite()
  circle = script.entity:addCircle();
  body = script.entity:addRigidBody2D()
  local box = script.entity:addBoxCollider()
  move = script.entity:addMovment();
  

  body.gravity_scale = 3
  box.friction = 0
  box.restitution = 0.5
  --box.restitution_threshold = 2
  box.density = 0.2 
  trans = script.entity:getTransform()
  --print(script.entity:insertRigidBody2D())
  --box = script.entity:insertBoxCollider()
  --box.restitution = 1;
  --print("first color: ", sprite.color);
  --print(script.entity:getUUID());
  --local sprite = script.entity:getSprite()
 -- print("sprite: ", sprite)
  print("done getting sprite")
 -- sprite.color = Real.Math.Vec4.new();
  --sprite.color.x = 0.1;
  --sprite.color.y = 0.5;
  --sprite.color.z = 0.1;
  --sprite.color.w = 1.0;

  print("getting trans")
  --trans = script.entity:insertTransform()
  --trans.rotation = Real.Vec4.new()
  print("done getting trans")

  --trans.position.x = Real.Vec4.new()
  --trans.position.y = Real.Vec4.new()
  --trans.rotation.z = 
  --print(vec1 + vec2)
  print("exited onCreate")
end
local num = 0
x = true
function script:onUpdate()
  --print("Scene*");
  print("===================")
  print("karen: ", karen)
  print("===================")
  if(true) then
  if(Real.Input.isKeyPressed(Real.Keyboard.W)) then
  move.linear_velocity.y = 5
  print("W")
  elseif(Real.Input.isKeyPressed(Real.Keyboard.S)) then
  move.linear_velocity.y = -5 
  print("S")
  elseif x then move.linear_velocity.y = lerp(move.linear_velocity.y, 0, 0.5)
  end
  
  if(Real.Input.isKeyPressed(Real.Keyboard.D)) then
  move.linear_velocity.x = 5
  print("D")
  elseif(Real.Input.isKeyPressed(Real.Keyboard.A)) then
  move.linear_velocity.x = -5
  print("A")
  elseif x then move.linear_velocity.x = lerp(move.linear_velocity.x, 0, 0.5)
  end
  end
  if(Real.Input.isKeyPressed(Real.Keyboard.E)) then 
    move.angular_velocity = lerp(move.angular_velocity, -3, 0.05)
  elseif(Real.Input.isKeyPressed(Real.Keyboard.Q)) then 
    move.angular_velocity = lerp(move.angular_velocity, 3, 0.05)
  elseif x then move.angular_velocity = lerp(move.angular_velocity, 0, 0.05)
  end

  --trans.position.x = math.sin(num) * 4
  --trans.position.y = math.cos(num) * 6
 -- trans.rotation.z = math.tan(num);

  local pos = Real.Vec3.new(0, 0, -0.1)
  --local rot = Real
  local size = Real.Vec2.new(0, 0)
  local color = Real.Vec4.new(0.3, 0.3, 0.8, 1.0)
--camera matrix 
  local cam_transform = Real.Mat4.new(1);
  cam_transform = cam_transform:translate(Real.Vec4.new(0))
  --
  --local _pos = pos:sub(Real.Vec3.new(size.x, size.y, 1):dev(2))
--  local cam_proj = scene_camera_entity:getCamera().camera:getProjection()
--  local cam_trans = scene_camera_entity:getTransform():asMat4()
  --Real.Renderer2D.beginScene(cam_proj, cam_trans)
  --Real.Renderer2D.drawQuad(pos, size, color)
  --Real.Renderer2D.endScene()
  local ts = script:getTimestep()
  circle.color = col;
  --sprite.color = col;
  --sprite.color.z = math.tan(num)
--  if not ts == nil then
  --  print(sprite.color)
    print(ts)
  --else print("ts is nill")
  --end
  --trans.rotation.z = trans.rotation.z + math.sin(num)


  num = num + 0.01;
  if num > 1 then
    --num = 0
  end
end

function script:onDestroy()
  --karen = "karen"
  --qais = "qais"
  print("onEnd")
  local v1 = Real.Vec4.new(2)
  local v2 = Real.Vec4.new(1)
  local m1 = Real.Mat4.new(2)
  local m2 = Real.Mat4.new(3)
  print(m1:mul(v2))
  print(v1:mul(v2))
  print(m1:translate(v1))
  print(v1:add(v2))
  print(m1:inverse())
end

print("made getobj")
function GetObject()
  print("GetObject Called")
  return script;
end
