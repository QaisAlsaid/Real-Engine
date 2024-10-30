local cscript = Real.Script.new()
local centity = cscript.entity
function cscript:onCreate()
  print("created entity id: ", centity:getId())
  centity = cscript.entity
end

function cscript:onUpdate()
  centity:getTransform().position = entity1:getTransform().position 
end

function cscript:onDestroy()
end

function GetObject()
  return cscript
end
