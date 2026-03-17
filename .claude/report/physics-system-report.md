# GTA San Andreas 物理引擎系统逆向工程报告

> 生成日期: 2026-03-16

## 1. 架构总览

GTA SA 没有使用第三方物理引擎（如 Havok/PhysX），而是**完全自研**的刚体脉冲物理系统。

```
┌─────────────────────────────────────────────────────┐
│                  CWorld (世界管理)                     │
│  ProcessLineOfSight, TestSphereAgainstWorld,          │
│  FindGroundZForCoord, 扇区遍历                        │
├─────────────────────────────────────────────────────┤
│              CPhysical (物理实体基类)                   │
│  力/扭矩累加 → 碰撞检测/响应 → 速度积分 → 位置更新      │
├───────────────────┬─────────────────────────────────┤
│  CCollision       │  cBuoyancy                       │
│  几何碰撞检测      │  浮力系统                         │
│  (59 个静态方法)   │  (9 个方法)                       │
├───────────────────┴─────────────────────────────────┤
│  碰撞原语: CColSphere, CColBox, CColTriangle,        │
│  CColLine, CColDisk, CColModel, CCollisionData       │
└─────────────────────────────────────────────────────┘
```

## 2. CPhysical — 物理核心

**文件**: `source/game_sa/Entity/Physical.h` / `Physical.cpp`（4760 行）
**结构体大小**: `VALIDATE_SIZE(CPhysical, 0x138)` — 312 字节
**继承链**: `CPlaceable (0x18) → CEntity (0x38) → CPhysical (0x138)`

### 关键成员变量

#### 运动学变量

| 变量 | 类型 | 说明 |
|------|------|------|
| `m_vecMoveSpeed` | CVector | 线速度（世界单位/帧） |
| `m_vecTurnSpeed` | CVector | 角速度（弧度/帧/轴） |
| `m_vecFrictionMoveSpeed` | CVector | 摩擦线速度累积器 |
| `m_vecFrictionTurnSpeed` | CVector | 摩擦角速度累积器 |
| `m_vecForce` | CVector | 外力累积器 |
| `m_vecTorque` | CVector | 扭矩累积器 |

#### 质量/惯性参数

| 变量 | 类型 | 说明 |
|------|------|------|
| `m_fMass` | float | 平移质量（默认 1.0） |
| `m_fTurnMass` | float | 转动惯量（默认 1.0） |
| `m_fVelocityFrequency` | float | 速度频率因子（默认 1.0） |
| `m_fAirResistance` | float | 空气阻力系数（默认 0.1） |
| `m_fElasticity` | float | 恢复系数（弹性） |
| `m_fBuoyancyConstant` | float | 浮力常数 |
| `m_vecCentreOfMass` | CVector | 局部空间质心偏移 |

#### 碰撞状态

| 变量 | 类型 | 说明 |
|------|------|------|
| `m_nLastCollisionTime` | uint32 | 上次碰撞时间戳（ms） |
| `m_apCollidedEntities[6]` | CEntity*[] | 本帧碰撞实体缓存 |
| `m_nNumEntitiesCollided` | uint8 | 碰撞实体数量 |
| `m_nContactSurface` | eSurfaceType | 当前接触表面类型 |
| `m_fDamageIntensity` | float | 本帧最大碰撞伤害强度 |
| `m_pDamageEntity` | CEntity* | 造成最大伤害的实体 |
| `m_vecLastCollisionImpactVelocity` | CVector | 上次碰撞冲击速度 |
| `m_vecLastCollisionPosn` | CVector | 上次碰撞世界位置 |
| `m_fMovingSpeed` | float | 本帧移动标量距离 |

#### 附着系统

| 变量 | 类型 | 说明 |
|------|------|------|
| `m_pAttachedTo` | CPhysical* | 附着到的父实体 |
| `m_vecAttachOffset` | CVector | 局部偏移 |
| `m_vecAttachedEntityRotation` | CVector | 欧拉旋转偏移 |
| `m_qAttachedEntityRotation` | CQuaternion | 四元数旋转偏移 |
| `m_pEntityIgnoredCollision` | CEntity* | 忽略碰撞的实体（脱离后使用） |

#### 光照

| 变量 | 类型 | 说明 |
|------|------|------|
| `m_fContactSurfaceBrightness` | float | 表面亮度贡献 |
| `m_fDynamicLighting` | float | 动态光照量 |
| `m_pShadowData` | CRealTimeShadow* | 实时阴影数据 |

### 物理标志位（32 位位域）

| 标志 | 说明 |
|------|------|
| `bMakeMassTwiceAsBig` | 碰撞中双倍有效质量 |
| `bApplyGravity` | 每帧施加重力（默认 true） |
| `bDisableCollisionForce` | 不接收碰撞冲量 |
| `bCollidable` | 实体有碰撞 |
| `bDisableTurnForce` | 无角冲量（行人、部分物体） |
| `bDisableMoveForce` | 无平移冲量 |
| `bInfiniteMass` | 无限平移质量 |
| `bDisableZ` | 约束 Z 轴移动（台球、2D 物体） |
| `bSubmergedInWater` | 当前在水下 |
| `bOnSolidSurface` | 接触固体表面 |
| `bBroken` | 实体已破碎 |
| `bProcessCollisionEvenIfStationary` | 静止时也强制碰撞 |
| `bSkipLineCol` | 跳过线段碰撞（仅行人） |
| `bDontApplySpeed` | 不将速度应用到位置 |
| `bProcessingShift` | 当前在位移修正阶段 |
| `bDisableSimpleCollision` | 跳过简化碰撞路径 |
| `bBulletProof` | 免疫子弹 |
| `bFireProof` | 免疫火焰 |
| `bCollisionProof` | 免疫碰撞伤害 |
| `bMeleeProof` | 免疫近战 |
| `bInvulnerable` | 完全无敌 |
| `bExplosionProof` | 免疫爆炸 |
| `bDontCollideWithFlyers` | 跳过飞行器碰撞 |
| `bAttachedToEntity` | 当前附着到其他实体 |
| `bAddMovingCollisionSpeed` | 碰撞中包含移动组件速度 |
| `bTouchingWater` | 部分在水中 |
| `bCanBeCollidedWith` | 维护碰撞实体列表 |
| `bRenderScorched` | 焦化视觉效果 |

### 静态物理常量

| 常量 | 地址 | 说明 |
|------|------|------|
| `DAMPING_LIMIT_IN_FRAME` | 0x8CD7A0 | 帧内最大阻尼力 |
| `DAMPING_LIMIT_OF_SPRING_FORCE` | 0x8CD7A4 | 弹簧力阻尼上限 |
| `PHYSICAL_SHIFT_SPEED_DAMP` | 0x8CD788 | 位移修正速度阻尼因子 |
| `SOFTCOL_SPEED_MULT` / `MULT2` | 0x8CD794 / 0x8CD798 | 软碰撞速度乘数 |
| `SOFTCOL_DEPTH_MIN` / `MULT` | 0x8CD78C / 0x8CD790 | 软碰撞深度参数 |
| `SOFTCOL_CARLINE_SPEED_MULT` | 0x8CD79C | 车辆线段软碰撞乘数 |
| `HIGHSPEED_ELASTICITY_MULT_COPCAR` | 0x8CD784 | 警车高速弹性乘数 |

### 物理更新管线（每帧执行）

```
ProcessControl()         ← 1. 力/扭矩累加（重力、空气阻力、引擎力等）
  ├─ ApplyFriction()            ← 冲刷摩擦累积器到速度
  ├─ ApplyGravity()             ← m_vecMoveSpeed.z -= dt * 0.008f
  ├─ ApplyAirResistance()       ← 指数衰减拖曳力
  └─ 子类特定力（悬挂、引擎、浮力...）

ProcessCollision()       ← 2. 碰撞检测与响应
  ├─ STATUS_SIMPLE 路径:
  │   └─ CheckCollision_SimpleCar()  ← 简化碰撞，碰到则切换到 STATUS_PHYSICS
  ├─ STATUS_GHOST 路径:
  │   └─ 仅轮子线段碰撞 vs 幽灵路面高度
  └─ 正常路径:
      ├─ 计算 collisionSteps（子步数）
      ├─ 对每个子步: ApplySpeed() + CheckCollision()
      │   ├─ ProcessCollisionSectorList()  ← 遍历世界扇区
      │   │   ├─ ProcessEntityCollision()  ← 单实体对碰撞
      │   │   │   └─ CCollision::ProcessColModels()  ← 几何碰撞检测
      │   │   ├─ ApplyCollision()          ← 刚体碰撞冲量响应
      │   │   ├─ ApplySoftCollision()      ← 软碰撞（穿透修正）
      │   │   └─ ApplyFriction()           ← 碰撞摩擦力
      │   └─ SetDamagedPieceRecord()       ← 记录最大伤害
      └─ 最终 ApplySpeed() + Reorthogonalise() + RemoveAndAdd()

ProcessShift()           ← 3. 位移修正（低速实体简化处理）
  ├─ ApplySpeed()
  ├─ ProcessShiftSectorList()   ← 查找穿透
  │   └─ 按 colPoint.m_fDepth 比例施加位移向量
  └─ 可选第二轮 ProcessCollisionSectorList()
```

### 关键物理方法

#### 力施加

| 方法 | 地址 | 说明 |
|------|------|------|
| `ApplyMoveForce(CVector)` | 0x5429F0 | `m_vecMoveSpeed += force / m_fMass` |
| `ApplyTurnForce(force, point)` | 0x542A50 | `m_vecTurnSpeed += Cross(point-CoM, force) / m_fTurnMass` |
| `ApplyForce(force, point, bool)` | 0x542B50 | 组合线性+角冲量 |
| `ApplyGravity()` | 0x542FE0 | `m_vecMoveSpeed.z -= timeStep * 0.008f` |
| `ApplyAirResistance()` | 0x544C40 | 对移动+转向速度施加指数衰减拖曳 |
| `ApplyFrictionMoveForce()` | 0x5430A0 | 累积到 `m_vecFrictionMoveSpeed` |
| `ApplyFrictionTurnForce()` | — | 累积到 `m_vecFrictionTurnSpeed` |
| `ApplyFrictionForce()` | 0x543220 | 同时累积线性和角摩擦 |

#### 碰撞响应

| 方法 | 地址 | 说明 |
|------|------|------|
| `ApplyCollision(entity, colPoint, damage&)` | 0x5435C0 | 单体碰撞冲量（vs 静态） |
| `ApplyCollision(entity, colPoint, thisD&, entityD&)` | 0x548680 | 双体碰撞，含连根拔起 |
| `ApplyCollisionAlt(physical, colPoint, ...)` | 0x544D50 | 双体碰撞，弹性分支 |
| `ApplySoftCollision(entity, colPoint, damage&)` | 0x543890 | 基于深度的软冲量 |
| `ApplySoftCollision(physical, colPoint, thisD&, entityD&)` | 0x54A2C0 | 双体软碰撞 |
| `ApplyFriction(float, CColPoint&)` | 0x5454C0 | 表面自摩擦 |
| `ApplyFriction(CPhysical*, float, CColPoint&)` | 0x545980 | 双体摩擦 |
| `ApplyScriptCollision()` | — | 脚本碰撞 |

#### 弹簧/悬挂

| 方法 | 地址 | 说明 |
|------|------|------|
| `ApplySpringCollision(...)` | 0x543C90 | `F = stress * mass * level * 0.016 * dt * bias` |
| `ApplySpringCollisionAlt(...)` | 0x543D60 | 带法线的弹簧力（汽车主路径） |
| `ApplySpringDampening(...)` | 0x543E90 | 弹簧阻尼，速度投影 + 钳制 |

弹簧公式:
```
stress = 1.0 - springLength   (0=完全伸展, 1=完全压缩)
if stress <= 0: 弹簧伸展，无力
limit = stress * mass * suspForceLevel * 0.016 * dt * bias
ApplyForce(-limit * direction, collisionPoint)
```

阻尼公式:
```
dampSpeed = -(dt * dampForce) * dot(collPos, direction)
clamp dampSpeed to [-DAMPING_LIMIT_IN_FRAME, +DAMPING_LIMIT_IN_FRAME]
effectiveMass = GetMass(distance_from_CoM, direction)
springForceDamping = effectiveMass * dampSpeed
clamp to |springForceDampingLimit| * DAMPING_LIMIT_OF_SPRING_FORCE
ApplyForce(damping * direction, collisionPoint)
```

#### 速度积分与查询

| 方法 | 地址 | 说明 |
|------|------|------|
| `ApplyMoveSpeed()` | 0x542DD0 | `position += timeStep * m_vecMoveSpeed` |
| `ApplyTurnSpeed()` | 0x542E20 | 按 `m_vecTurnSpeed * timeStep` 旋转矩阵轴 |
| `ApplySpeed()` | 0x547B80 | 组合 ApplyMoveSpeed + ApplyTurnSpeed（含台球桌边界处理） |
| `GetSpeed(CVector point)` | 0x542CE0 | 指定世界点的速度（线性+角贡献） |
| `GetMass(CVector pos, CVector dir)` | — | 有效碰撞质量: `1 / (|r×n|²/turnMass + 1/mass)` |
| `SkipPhysics()` | 0x5433B0 | 标记帧为跳过物理 |

#### 碰撞检测循环

| 方法 | 地址 | 说明 |
|------|------|------|
| `CheckCollision()` | 0x54D920 | 全碰撞遍历：Buildings, Vehicles, Peds, Objects |
| `CheckCollision_SimpleCar()` | 0x54DAB0 | STATUS_SIMPLE 简化碰撞 |
| `ProcessCollisionSectorList()` | 0x54BA60 | 扇区级碰撞处理循环 |
| `ProcessCollisionSectorList_SimpleCar()` | 0x54CFF0 | 简化扇区碰撞 |
| `ProcessShiftSectorList()` | 0x546670 | 扇区级位移修正 |

#### 附着与其他

| 方法 | 说明 |
|------|------|
| `AttachEntityToEntity(CVector, CVector)` | 以欧拉旋转附着 |
| `AttachEntityToEntity(CVector*, CQuaternion*)` | 以四元数附着 |
| `DettachEntityFromEntity(x, y, z, bool)` | 脱离，可选施加抛力 |
| `PositionAttachedEntity()` | 更新附着实体变换 |
| `AddCollisionRecord(CEntity*)` | 记录碰撞，设置 bOnSolidSurface |
| `SetDamagedPieceRecord(...)` | 更新伤害记录（如果更严重） |
| `CanPhysicalBeDamaged(eWeaponType, bool*)` | 武器+标志伤害门控 |
| `UnsetIsInSafePosition()` | 回退到上一个安全位置 |
| `PlacePhysicalRelativeToOtherPhysical()` | 静态方法，驾校任务使用 |

### 虚方法表（VMT 0x863BA0，23 个槽位）

| 方法 | 地址 | 状态 | 说明 |
|------|------|------|------|
| `Add()` | 0x544A30 | 未逆向 | 世界扇区注册 |
| `Remove()` | 0x5424C0 | 已逆向 | 世界扇区移除 |
| `GetBoundRect()` | 0x5449B0 | 已逆向 | 返回包围矩形 |
| `ProcessControl()` | 0x5485E0 | 已逆向 | 每帧预物理 |
| `ProcessCollision()` | 0x54DFB0 | 已逆向 | 主碰撞解算 |
| `ProcessShift()` | 0x54DB10 | 已逆向 | 位移修正 |
| `TestCollision(bool)` | 0x54DEC0 | 已逆向 | 仅测试碰撞 |
| `ProcessEntityCollision(CEntity*, CColPoint*)` | 0x546D00 | 已逆向 | 单实体对碰撞 |

## 3. CCollision — 碰撞检测

**文件**: `source/game_sa/Collision/Collision.h` / `Collision.cpp`（3587 行，59 个静态方法）

纯静态类，实现所有几何碰撞检测算法。

### 碰撞测试（Test* — 仅返回 bool）

| 方法 | 说明 |
|------|------|
| `TestSphereSphere()` | 球-球相交 |
| `TestSphereBox()` | 球-AABB 相交 |
| `TestSphereTriangle()` | 球-三角形相交 |
| `TestLineSphere()` | 线段-球相交 |
| `TestLineBox()` / `TestLineBox_DW()` | 线段-AABB 相交 |
| `TestVerticalLineBox()` | 垂直线段-AABB |
| `TestLineTriangle()` | 线段-三角形 |
| `TestLineOfSight()` | 视线测试（含穿透检查） |
| `Test2DLineAgainst2DLine()` | 2D 线段相交 |

### 碰撞处理（Process* — 返回碰撞点信息）

| 方法 | 说明 |
|------|------|
| `ProcessSphereSphere()` | 球-球碰撞点 |
| `ProcessSphereBox()` | 球-AABB 碰撞点 |
| `ProcessSphereTriangle()` | 球-三角形碰撞点 |
| `ProcessLineSphere()` | 线段-球碰撞点 |
| `ProcessLineBox()` | 线段-AABB 碰撞点 |
| `ProcessLineTriangle()` | 线段-三角形碰撞点 |
| `ProcessVerticalLineTriangle()` | 垂直线段-三角形 |
| `ProcessLineOfSight()` | 视线处理 |
| `ProcessVerticalLine()` | 垂直线碰撞 |
| `ProcessColModels()` | **核心**: 两个 CColModel 完整碰撞检测，返回碰撞点数（最多 32 个球碰撞点） |
| `ProcessDiscCollision()` | 圆盘碰撞 |

### 距离/几何辅助

| 方法 | 说明 |
|------|------|
| `DistToLine()` / `DistToLineSqr()` | 点到线距离 |
| `DistToMathematicalLine()` / `2D` | 点到数学线距离 |
| `DistAlongLine2D()` | 点沿线投影距离 |
| `ClosestPointOnLine()` | 线上最近点 |
| `ClosestPointsOnPoly()` / `ClosestPointOnPoly()` | 多边形上最近点 |
| `PointInTriangle()` / `PointInPoly()` | 点在多边形内测试 |
| `GetPrincipleAxis()` | 获取主轴 |

### 球体扫掠（SphereCast）

| 方法 | 说明 |
|------|------|
| `SphereCastVsSphere()` | 球-球扫掠（**唯一已逆向的方法**） |
| `SphereCastVsBBox()` | 球-AABB 扫掠 |
| `SphereCastVsEntity()` | 球-实体扫掠 |
| `SphereCastVsCaches()` | 球-缓存扫掠 |
| `SphereCastVersusVsPoly()` | 球-多边形扫掠 |
| `SphereVsEntity()` | 球-实体测试 |

### 相机碰撞

| 方法 | 说明 |
|------|------|
| `CheckCameraCollisionBuildings()` | 相机-建筑碰撞 |
| `CheckCameraCollisionVehicles()` | 相机-载具碰撞 |
| `CheckCameraCollisionObjects()` | 相机-物体碰撞 |
| `CheckCameraCollisionPeds()` | 相机-行人碰撞 |
| `BuildCacheOfCameraCollision()` | 构建相机碰撞缓存 |
| `CameraConeCastVsWorldCollision()` | 相机锥体碰撞 |

### NOTSA 辅助方法（项目自行添加）

- `GetClosestPtOnLine()`, `GetBaryCoordsOnTriangle()`
- `GetClampedBaryCoordsIntoTriangle()`, `GetCoordsClampedIntoTriangle()`
- `ClosestPtSegmentSegment()`

### 调试设置

`CCollision` 包含一个 `DebugSettings` 结构，允许按形状对启用/禁用碰撞测试（4x4 矩阵：Box/Sphere/Triangle/Line），以及控制线段起点在球内部时是否算相交。

## 4. 碰撞原语系统

**目录**: `source/game_sa/Collision/`（20 个头文件）

### 碰撞模型架构

```
CColModel (0x30, 48 字节) — 附着到每个实体的复合碰撞模型
  ├─ m_boundBox (CBoundingBox) — AABB 快速排除
  ├─ m_boundSphere (CSphere) — 球体快速排除
  ├─ slot, flags
  └─ CCollisionData* (0x30, 48 字节) — 详细几何
       ├─ CColSphere[]   — 包围球数组
       ├─ CColBox[]      — AABB 数组
       ├─ CColTriangle[] — 三角形网格
       │   └─ (可选 TFaceGroup[] 在三角形前，如果 bHasFaceGroups)
       ├─ CColLine[]     — 线段（用于轮子等）
       │   └─ 或 CColDisk[]（如果 bUsesDisks）
       ├─ CompressedVector[] — 压缩顶点池
       ├─ CColTrianglePlane[] — 预计算平面（与三角形同数）
       └─ 阴影网格: m_pShadowVertices[], m_pShadowTriangles[]
```

### 原语类型和大小

| 类名 | 大小 | 成员 |
|------|------|------|
| `CColPoint` | 0x2C (44B) | m_vecPoint, m_vecNormal, m_nSurfaceTypeA/B, m_nPieceTypeA/B, m_nLightingA/B, m_fDepth |
| `CColSphere` | 0x14 (20B) | CSphere (center+radius) + CColSurface (material+piece+lighting) |
| `CColBox` | 0x1C (28B) | CBox (min+max) + CColSurface |
| `CColLine` | 0x20 (32B) | m_vecStart, m_fStartSize, m_vecEnd, m_fEndSize |
| `CColTriangle` | 0x08 (8B) | vA/vB/vC (uint16 顶点索引) + m_nMaterial + m_nLight |
| `CColTrianglePlane` | 0x0A (10B) | 压缩单位法线 + FixedFloat<int16,128f> 偏移 + Orientation 枚举 |
| `CColModel` | 0x30 (48B) | CBoundingBox, CSphere, slot, flags, CCollisionData* |
| `CCollisionData` | 0x30 (48B) | 计数+指针：球/盒/线/顶点/三角形/平面/阴影 |
| `CColDisk` | — | 3D 空间中的圆形 |
| `CBoundingBox` | — | min/max 角点 |

### .col 文件格式版本

- **COLL (v1)**: 未压缩顶点
- **COL2**: 压缩顶点
- **COL3**: +面组 (face groups)
- **COL4**: +阴影网格 (shadow mesh)

### CColStore — 碰撞流式加载管理器

管理按世界区域加载/卸载 `.col` 文件。关键方法（全部已逆向）：
- `AddColSlot(name)` — 注册碰撞区域
- `LoadCol(slot, filename)` / `LoadCol(slot, data, size)` — 加载碰撞几何
- `LoadCollision(pos, ignorePlayerVeh)` — 基于玩家位置加载
- `EnsureCollisionIsInMemory(pos)` — 流式按需加载
- `RequestCollision(pos, areaCode)` — 请求加载
- `AddRef()` / `RemoveRef()` — 引用计数

## 5. cBuoyancy — 浮力系统

**文件**: `source/game_sa/Buoyancy.h` / `Buoyancy.cpp`（477 行）
**大小**: `VALIDATE_SIZE(cBuoyancy, 0xD0)` — 208 字节
**全局单例**: `extern cBuoyancy& mod_Buoyancy;`（地址 0xC1C890）

### 浮力状态

| 成员 | 说明 |
|------|------|
| `m_vecPos` | 正在计算浮力的实体位置 |
| `m_EntityMatrix` | 实体变换矩阵 |
| `m_fWaterLevel` | 实体位置处的水面 Z 坐标 |
| `m_fBuoyancy` | 计算的浮力大小 |
| `m_vecBoundingMax/Min` | 实体边界（从碰撞边界修改） |
| `m_fNumCheckedPoints` | 增量平均分母（最多检查 9 个点） |
| `m_bInWater` | 当前在水中 |
| `m_bProcessingBoat` | 船只特殊路径 |
| `m_fEntityWaterImmersion` | [0,1] 实体浸入水中的比例 |
| `m_vecTurnPoint` | 局部空间浮力施加点 |

### 船只体积分布矩阵（3x3 网格）

| 静态数组 | 说明 |
|----------|------|
| `afBoatVolumeDistribution[3][3]` | 标准船只 |
| `afBoatVolumeDistributionSail[3][3]` | 帆船 |
| `afBoatVolumeDistributionDinghy[3][3]` | 小艇 |
| `afBoatVolumeDistributionSpeed[3][3]` | 快艇 |
| `afBoatVolumeDistributionCat[3][3]` | 双体船（未使用，无对应载具） |

### 方法

| 方法 | 说明 |
|------|------|
| `ProcessBuoyancy(CPhysical*, float, CVector*, CVector*)` | 通用实体浮力处理 |
| `ProcessBuoyancyBoat(CVehicle*, float, CVector*, CVector*, bool)` | 船只专用浮力 |
| `CalcBuoyancyForce(CPhysical*, CVector*, CVector*)` | 计算浮力向量 |
| `PreCalcSetup(CPhysical*, float)` | 预计算设置 |
| `SimpleCalcBuoyancy(CPhysical*)` | 简化浮力计算 |
| `SimpleSumBuoyancyData(CVector*, tWaterLevel)` | 累加浮力数据 |
| `FindWaterLevel(CVector&, CVector*, tWaterLevel*)` | 水面高度查询 |
| `FindWaterLevelNorm(CVector&, CVector*, tWaterLevel*, CVector*)` | 带法线的水面查询 |
| `AddSplashParticles(CPhysical*, CVector, CVector, CVector, uint8)` | 入水飞溅粒子 |

浮力被 `Boat.cpp`, `Object.cpp`, `Ped.cpp`, `Projectile.cpp`, `Automobile.cpp`, `Vehicle.cpp` 调用。

### tWaterLevel 枚举

```cpp
COMPLETELY_ABOVE_WATER = 0  // 完全在水面上
COLLIDING_WITH_WATER   = 1  // 与水面碰撞
COMPLETELY_UNDER_WATER = 2  // 完全在水下
```

## 6. CWorld — 世界物理查询

**文件**: `source/game_sa/World.h/cpp`（93 个方法）

### 射线查询

| 方法 | 说明 |
|------|------|
| `ProcessLineOfSight(origin, target, ...)` | 射线与世界碰撞，可按类型过滤（建筑/载具/行人/物体/假人） |
| `ProcessLineOfSightSector(...)` | 扇区级射线碰撞 |
| `ProcessLineOfSightSectorList<T>(...)` | 模板化链表级射线碰撞 |
| `ProcessVerticalLine(origin, dist, ...)` | 垂直射线碰撞 |
| `ProcessVerticalLineSector(...)` | 扇区级垂直射线 |

### 球体/区域查询

| 方法 | 说明 |
|------|------|
| `TestSphereAgainstWorld(center, radius, ...)` | 球体与世界碰撞测试 |
| `TestSphereAgainstSectorList(...)` | 球体与扇区链表测试 |
| `FindObjectsInRange(...)` | 范围内物体查询 |
| `FindObjectsKindaColliding(...)` | 近似碰撞查询 |
| `FindObjectsIntersectingCube(...)` | 立方体相交查询 |

### 地面高度查询

| 方法 | 说明 |
|------|------|
| `FindGroundZForCoord(x, y)` | 指定 XY 处的地面 Z 坐标 |
| `FindGroundZFor3DCoord(coord, outResult, outEntity)` | 3D 坐标处地面高度 |

所有方法均遍历世界扇区网格，调用 `CCollision::ProcessLineOfSight` 或 `CCollision::SphereVsEntity`。

## 7. 载具特定物理

载具的物理模拟建立在 CPhysical 之上：

| 子系统 | 位置 | 说明 |
|--------|------|------|
| **悬挂弹簧** | CAutomobile/CBike | `ApplySpringCollision()` + `ApplySpringDampening()` |
| **轮胎摩擦** | CAutomobile | 4 轮独立摩擦，地面材质相关 |
| **飞行控制** | CVehicle | `FlyingControl()` — 直升机/飞机气动力 |
| **操控数据** | tHandlingData | 质量、牵引力、刹车力、变速箱 |
| **浮力** | cBuoyancy | 船只和落水载具 |
| **碰撞伤害** | CAutomobile | `VehicleDamage()` — 碰撞强度到部件损坏的映射 |

### 悬挂参数（tHandlingData）

```cpp
float m_fSuspensionForceLevel;       // 弹簧刚度
float m_fSuspensionDampingLevel;     // 阻尼系数
float m_fSuspensionHighSpdComDamp;   // 高速压缩阻尼
float m_fSuspensionUpperLimit;       // 最大伸展
float m_fSuspensionLowerLimit;       // 最小压缩（负值）
float m_fSuspensionBiasBetweenFrontAndRear;  // 前后偏置
float m_fSuspensionAntiDiveMultiplier;       // 防俯冲乘数
```

## 8. ApplyMoveForce/ApplyTurnForce 调用者分布

在 36+ 个文件中被调用，包括：
- **载具**: Vehicle.cpp, Bike.cpp, Automobile.cpp, Boat.cpp
- **行人**: Ped.cpp
- **物体**: Object.cpp, Projectile.cpp
- **武器**: BulletInfo.cpp, WaterCannon.cpp
- **任务**: TaskSimpleJump, TaskSimpleInAir, TaskSimpleSwim, TaskSimpleJetPack
- **事件**: EventDamage, EventKnockOffBike

## 9. 逆向工程进度

| 模块 | 总函数 | 已逆向 | 完成率 | 说明 |
|------|--------|--------|--------|------|
| **CPhysical** | 56 | 54 | **96.4%** | 仅 `Add()`, `RemoveAndAdd()` 未完成 |
| **CCollision** | 59 | **1** | **1.7%** | 几乎全部未逆向！58 个函数锁定到原始代码 |
| **CWorld** | 93 | 92 | **98.9%** | 仅 `CallOffChaseForAreaSectorListPeds` 未完成 |
| **CColModel** | 9 | 9 | 100% | 完全完成 |
| **CColStore** | 23 | 23 | 100% | 完全完成 |
| **CColAccel** | 11 | 11 | 100% | 完全完成 |
| **CCollisionData** | 8 | 8 | 100% | 完全完成 |
| **CColSphere** | 5 | 5 | 100% | 完全完成 |
| **CColLine** | 1 | 1 | 100% | 完全完成 |
| **CTempColModels** | 2 | 2 | 100% | 完全完成 |
| **cBuoyancy** | 9 | 9 | 100% | 完全完成 |
| **合计** | **276** | **215** | **77.9%** | |

### 关键发现：CCollision 是项目中最大的未逆向子系统

`CCollision` 的 59 个碰撞检测函数中**仅 1 个被逆向**（`SphereCastVsSphere`），其余 58 个全部 `locked=1`，锁定运行原始 GTA 代码：

- 所有球-三角形、线-三角形、球-球等几何碰撞算法
- `ProcessColModels()`（两个碰撞模型的完整交叉测试）
- 相机碰撞系统
- 视线检测算法
- 3587 行的 `Collision.cpp` 基本上是空壳

## 10. 代码量统计

| 文件 | 行数 |
|------|------|
| Physical.cpp | 4,760 |
| Collision.cpp | 3,587 |
| Buoyancy.cpp | 477 |
| ColStore.cpp | 468 |
| CollisionData.cpp | 271 |
| TempColModels.cpp | 237 |
| ColAccel.cpp | 218 |
| ColModel.cpp | 165 |
| ColourSet.cpp | 157 |
| Box.cpp | 107 |
| ColSphere.cpp | 91 |
| ColTrianglePlane.cpp | 61 |
| Sphere.cpp | 56 |
| ColLine.cpp | 34 |
| ColTriangle.cpp | 33 |
| ColBox.cpp | 20 |
| BoundingBox.cpp | 18 |
| ColPoint.cpp | 12 |
| CollisionEventScanner.cpp | 8 |
| **合计** | **~10,783** |

## 11. 总结

本项目**深度涉及物理引擎**，GTA SA 使用完全自研的刚体脉冲物理系统，包含以下层次：

1. **运动学层**（CPhysical）: 维护线速度 `m_vecMoveSpeed` 和角速度 `m_vecTurnSpeed`，每帧通过 `ApplySpeed()` 积分 — **96.4% 完成**
2. **力累加层**: `ApplyMoveForce/TurnForce/Force` 按 `1/mass` 和 `1/turnMass` 缩放冲量。接触点与质心的叉积产生角冲量
3. **碰撞检测层**（CCollision）: `ProcessColModels` 测试 CColModel 对，产生最多 32 个 CColPoint — **仅 1.7% 完成（最大缺口）**
4. **碰撞响应层**: `ApplyCollision/ApplySoftCollision/ApplyCollisionAlt` 使用弹性系数和有效碰撞质量公式: `m_eff = 1 / (|r×n|²/turnMass + 1/mass)`
5. **弹簧悬挂层**: `ApplySpringCollision[Alt]` + `ApplySpringDampening` 实现线性弹簧阻尼器，参数来自 `tHandlingData`
6. **浮力层**（cBuoyancy）: 在实体包围盒上采样 9 个点对水面，计算浸入比例和扭矩点 — **100% 完成**
7. **世界查询层**（CWorld）: 射线/球体/立方体碰撞查询 — **98.9% 完成**
8. **摩擦层**: 分两阶段施加 — 碰撞阶段累积到 `m_vecFrictionMoveSpeed/TurnSpeed`，下一帧 `ProcessControl()` 开始时冲刷到实际速度
