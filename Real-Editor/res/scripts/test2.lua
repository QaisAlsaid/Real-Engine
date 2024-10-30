local test2 = {}
local script = Real.Script.new()
local entity = script.entity
local num = 0
function script:onCreate()
  print("created entity id: ", entity:getId())
  entity = script.entity
end

c = 100
function script:onUpdate()
  c = c + 1
  t = script:getTimestep():getTime()
  print(math.sin(t))
  entity:getSprite().color = Real.Vec4.new(1)--( math.sin(t) * 100 ) - 0.6
end

function script:onDestroy()
end

function GetObject()
  return script
end
