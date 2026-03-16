# GTA San Andreas 载具系统逆向工程报告

> 生成日期: 2026-03-16

## 1. 类层次结构

```
CPlaceable (0x18)
  └─ CEntity (0x38)
       └─ CPhysical (0x138)
             └─ CVehicle (0x5A0) ─── 载具基类，1440 字节
                   ├─ CAutomobile (0x988) ─── 汽车，2440 字节
                   │     ├─ CHeli (0xA18) ─── 直升机，2584 字节
                   │     ├─ CPlane (0xA04) ─── 飞机，2564 字节
                   │     ├─ CMonsterTruck (0x99C) ─── 怪兽卡车，2460 字节
                   │     ├─ CQuadBike (0x9BC) ─── 四轮摩托，2492 字节
                   │     └─ CTrailer (0x9F4) ─── 拖车，2548 字节
                   ├─ CBike (0x814) ─── 摩托车，2068 字节
                   │     └─ CBmx (0x838) ─── BMX自行车，2104 字节
                   ├─ CBoat (0x7E8) ─── 船，2024 字节
                   └─ CTrain (0x6AC) ─── 火车，1708 字节
```

共 11 个载具类，分布在 `source/game_sa/Entity/Vehicle/` 下的 22 个文件（11 .h + 11 .cpp）中。

注意: CHeli 和 CPlane 继承自 CAutomobile（非直接继承 CVehicle），因此它们继承了汽车的全部门/轮子/悬挂基础设施。

### 结构体大小汇总

| 类名 | 大小 | 继承自 | 增量 |
|------|------|--------|------|
| CPlaceable | 0x018 | - | - |
| CEntity | 0x038 | CPlaceable | +0x020 |
| CPhysical | 0x138 | CEntity | +0x100 |
| CVehicle | 0x5A0 | CPhysical | +0x468 |
| CAutomobile | 0x988 | CVehicle | +0x3E8 |
| CHeli | 0xA18 | CAutomobile | +0x090 |
| CPlane | 0xA04 | CAutomobile | +0x07C |
| CMonsterTruck | 0x99C | CAutomobile | +0x014 |
| CQuadBike | 0x9BC | CAutomobile | +0x034 |
| CTrailer | 0x9F4 | CAutomobile | +0x06C |
| CBike | 0x814 | CVehicle | +0x274 |
| CBmx | 0x838 | CBike | +0x024 |
| CBoat | 0x7E8 | CVehicle | +0x248 |
| CTrain | 0x6AC | CVehicle | +0x10C |

### 双类型系统

CVehicle 同时持有 `m_nVehicleType` 和 `m_nVehicleSubType`（均为 eVehicleType）。子类型是一个 hack，允许像 Vortex（气垫船）这样的载具同时表现为两种类型。

### eVehicleType 枚举值

| 值 | 常量 | 对应类 |
|----|------|--------|
| 0 | VEHICLE_TYPE_AUTOMOBILE | CAutomobile |
| 1 | VEHICLE_TYPE_MTRUCK | CMonsterTruck |
| 2 | VEHICLE_TYPE_QUAD | CQuadBike |
| 3 | VEHICLE_TYPE_HELI | CHeli |
| 4 | VEHICLE_TYPE_PLANE | CPlane |
| 5 | VEHICLE_TYPE_BOAT | CBoat |
| 6 | VEHICLE_TYPE_TRAIN | CTrain |
| 7 | VEHICLE_TYPE_FHELI | 无专用类（子类型 hack） |
| 8 | VEHICLE_TYPE_FPLANE | 无专用类（子类型 hack） |
| 9 | VEHICLE_TYPE_BIKE | CBike |
| 10 | VEHICLE_TYPE_BMX | CBmx |
| 11 | VEHICLE_TYPE_TRAILER | CTrailer |

## 2. 核心类概览

### CVehicle（基类）— Vehicle.h:184, 804 行头文件, 4594 行实现
- 继承自 CPhysical，包含所有载具共有的逻辑
- 关键成员: CAEVehicleAudioEntity, tHandlingData*, tFlyingHandlingData*, CAutoPilot
- 64 位载具标志位（bEngineOn, bLightsOn, bIsDamaged 等 60+ 个布尔位域）
- 乘客系统（最多 8 个座位）、伤害系统、升级系统（15 个部件槽位）

### CAutomobile — 最复杂的派生类，6511 行实现
- 包含悬挂系统、轮子碰撞、门/面板伤害管理器
- CDamageManager, CDoor[6], 液压系统, 3 个 CBouncingPanel

### CBike — 2068 字节, 631 行实现
- 倾斜矩阵、tBikeHandlingData, CRideAnimData, wheelie 标志

### CBoat — 2024 字节, 1108 行实现
- 螺旋桨角度/速度、浮力系统、tBoatHandlingData, FX 螺旋桨系统

### CHeli — 2584 字节, 352 行实现
- 旋翼节点、tHeliLight 探照灯、旋翼速度字段

### CPlane — 2564 字节, 450 行实现
- 副翼/升降舵/起落架节点、高度追踪、螺旋桨速度

### CTrain — 1708 字节, 925 行实现
- 轨道距离、多车厢链表、乘客生成状态机（eTrainPassengersGenerationState）

## 3. 逆向工程进度

### 核心载具类 Hook 统计

| 类名 | 总函数 | 已逆向 | 未逆向 | 完成率 |
|------|--------|--------|--------|--------|
| CVehicle | 141 | 137 | 4 | 97.2% |
| CAutomobile | 99 | 97 | 2 | 98.0% |
| CBike | 40 | 21 | 19 | 52.5% |
| CBoat | 23 | 23 | 0 | 100% |
| CHeli | 11 | 11 | 0 | 100% |
| CPlane | 23 | 14 | 9 | 60.9% |
| CTrain | 40 | 20 | 20 | 50.0% |
| CBmx | 10 | 7 | 3 | 70.0% |
| CMonsterTruck | 11 | 5 | 6 | 45.5% |
| CQuadBike | 12 | 11 | 1 | 91.7% |
| CTrailer | 13 | 13 | 0 | 100% |
| **小计** | **423** | **359** | **64** | **84.9%** |

### 载具管理器/子系统 Hook 统计

| 类名 | 总函数 | 已逆向 | 未逆向 | 完成率 |
|------|--------|--------|--------|--------|
| CVehicleModelInfo | 75 | 75 | 0 | 100% |
| CAEVehicleAudioEntity | 106 | 102 | 4 | 96.2% |
| CCarEnterExit | 32 | 27 | 5 | 84.4% |
| CVehicleRecording | 29 | 29 | 0 | 100% |
| CCarAI | 25 | 25 | 0 | 100% |
| CCarCtrl | 22 | 21 | 1 | 95.5% |
| cHandlingDataMgr | 11 | 11 | 0 | 100% |
| CCarGenerator | 8 | 8 | 0 | 100% |
| **小计** | **308** | **298** | **10** | **96.8%** |

### 总计: 731 个函数，其中 657 个已逆向

## 4. 未逆向的关键函数

### CVehicle (4 个未逆向)
- `Load` / `Save` — 存档序列化（有意禁用，尚不可信）
- `SetVehicleAtomicVisibilityCB` / `RemoveObjectsCB` — RwObject 重载，锁定到原始代码

### CAutomobile (2 个未逆向)
- `Load` / `Save` — 存档序列化

### CBike (19 个未逆向 — 最大缺口之一)
- `ProcessControl`, `ProcessControlInputs`, `VehicleDamage`, `BurstTyre`, `BlowUpCar`
- `PreRender`, `SetupSuspensionLines`, `DoBurstAndSoftGroundRatios`, `SetUpWheelColModel`
- `PlayCarHorn`, `ProcessRiderAnims`, `FixHandsToBars`, `PlaceOnRoadProperly`
- `DoSoftGroundResistance`, `ResetSuspension`, `DamageKnockOffRider`
- `SetRemoveAnimFlags`, `ProcessAI`, `GetCorrectedWorldDoorPosition`

### CPlane (9 个未逆向)
- `ProcessControl`, `ProcessControlInputs`, `ProcessFlyingCarStuff`
- `VehicleDamage`, `BlowUpCar`, `PreRender`
- `SwitchAmbientPlanes`, `FindPlaneCreationCoors`, `DoPlaneGenerationAndRemoval`

### CTrain (20 个未逆向 — 最大缺口)
- `Constructor`, `InitTrains`, `ReadAndInterpretTrackFile`
- `CreateMissionTrain`, `DoTrainGenerationAndRemoval`
- `FindCoorsFromPositionOnTrack`, `FindMaximumSpeedToStopAtStations`
- `FindCaboose`, `FindEngine`, `FindCarriage`, `FindClosestTrackNode`
- `FindPositionOnTrackFromCoors`, `FindNearestTrain`
- `IsNextStationAllowed`, `SkipToNextAllowedStation`, `FindNextStationPositionInDirection`
- `RemoveAllTrains`, `RemoveRandomPassenger`, `AddNearbyPedAsRandomPassenger`
- `TrainHitStuff<CPtrListSingleLink<CPhysical*>>`

### CMonsterTruck (6 个未逆向)
- `ProcessControl`, `ProcessSuspension`, `ProcessControlCollisionCheck`
- `SetupSuspensionLines`, `ResetSuspension`, `ExtendSuspension`

### CBmx (3 个未逆向)
- `ProcessDrivingAnims`, `PreRender`, `ProcessAI`

### CQuadBike (1 个未逆向)
- `ProcessControl`

### CCarEnterExit (5 个未逆向)
- `GetNearestCarPassengerDoor`, `GetPositionToOpenCarDoor`
- `IsPlayerToQuitCarEnter`, `IsRoomForPedToLeaveCar`
- `MakeUndraggedPassengerPedsLeaveCar`

### CAEVehicleAudioEntity (4 个未逆向)
- `ProcessDummyBicycle`, `ProcessDummyHovercraft`
- `ProcessGenericJet`, `ProcessPlayerBicycle`

### CCarCtrl (1 个未逆向)
- `GetNewVehicleDependingOnCarModel`

### Vehicle.cpp 中 26 个注释掉的 hook（已定位地址但未开始逆向）
- `ProcessSirenAndHorn` (0x6E0950)
- `DoVehicleLights` (0x6E1A60)
- `DoTailLightEffect`
- `ProcessWeapons` (0x6E3950)
- `FireFixedMachineGuns`
- `DoDriveByShootings` (0x741FD0)
- `FlyingControl` (0x6D85F0)
- `UpdateTrailerLink` (0x6DFC50)
- `UpdateTractorLink` (0x6E0050)
- `FireHeatSeakingMissile`, `AddVehicleUpgrade` 等

## 5. 载具相关子系统全景

### 核心管理器（source/game_sa/）

| 文件 | 说明 |
|------|------|
| CarAI.h/cpp | 载具 AI 决策 — 警车任务分配、拦截/跟随/撞击指令、警笛让路（25 函数，100%） |
| CarCtrl.h/cpp | 交通管理 — 随机/紧急/任务载具生成、AI 转向（汽车/船/直升机/飞机）、车道切换、车队管理 |
| CarEnterExit.h/cpp | 上下车逻辑 — 门位置几何、拖拽、快/慢抢车（32 函数） |
| CarGenerator.h/cpp | 脚本载具生成器（8 函数，100%） |
| TheCarGenerators.h/cpp | 全局 500 个 CCarGenerator 实例管理器 |
| AutoPilot.h/cpp | 每载具自动驾驶状态 — 路径节点、eCarMission、eCarDrivingStyle、巡航速度 |
| VehicleRecording.h/cpp | 过场/任务录制回放系统 — 16 个同时播放槽位（29 函数，100%） |
| CurrentVehicle.h/cpp | 当前处理载具追踪 |
| CustomCarPlateMgr.h/cpp | 程序化车牌纹理生成（SF/LV/LA 地区样式） |
| VehicleScanner.h/cpp | 载具范围扫描（继承 CEntityScanner） |
| VehicleAnimGroupData.h/cpp | 载具驾驶动画组选择位域 |
| VehicleSaveStructure.h/cpp | 存档载具状态结构 |
| StuckCarCheck.h/cpp | 脚本注册载具卡住/翻转/淹没检测（最多 16 个） |
| LoadedCarGroup.h/cpp | 加权随机载具池，用于交通生成 |
| StoredCar.h | 可序列化载具快照 — 车库/存档用 |

### 操控数据

| 文件 | 说明 |
|------|------|
| tHandlingData.h | 通用操控参数 — 质量、转向质量、牵引力、cTransmission、悬挂、刹车 |
| tBikeHandlingData.h | 摩托车 — 倾斜速率、wheelie/stoppie 扭矩、最大倾斜角 |
| tBoatHandlingData.h | 船只 — 推进器乘数、舵力、尾流大小 |
| tFlyingHandlingData.h | 飞行载具 — 升力、推力、垂直升力、最大/失速高度 |
| cHandlingDataMgr.h/cpp | 操控数据管理器 — 加载/存储 210 个载具条目（11 函数，100%） |

### 模型信息 — Models/VehicleModelInfo.h/cpp
- CVehicleModelInfo（继承 CClumpModelInfo）— 75 个函数，100% 逆向
- 假人位置、升级附着点（16 槽位）、轮子模型/缩放、4 色调色板、生成频率权重

### 音频系统 — Audio/entities/AEVehicleAudioEntity.h/cpp
- CAEVehicleAudioEntity（继承 CAEAudioEntity）— 106 个函数，96.2% 逆向
- 引擎怠速/转速音效（带档位频率调制）
- 路面噪音、爆胎、打滑（表面依赖参数）
- 喇叭、警笛、碰撞、爆炸、水下音效
- 直升机旋翼音频（带倾斜因子）
- 相关枚举: eAEVehicleAudioType, eAEVehicleSoundType, eAEVehicleDoorType, eAEVehicleHornType

### 载具渲染管线
- Fx/CarFXRenderer.h/cpp — 载具自定义 RenderWare 管线：16 种灰尘纹理、环境贴图
- Pipelines/CustomCarEnvMap/ — 车漆环境贴图/高光贴图/大气着色（3 种材质类型）

### 载具相关任务 — Tasks/TaskTypes/（89 个任务类）

**驾驶任务:**
- TaskComplexCarDrive — 带速度/任务/风格参数驾驶
- TaskComplexCarDriveMission — 朝任务目标驾驶
- TaskComplexDriveToPoint, TaskComplexDriveWander, TaskComplexDriveFireTruck
- TaskComplexCopInCar — 警车中的警察行为

**进出车辆:**
- TaskComplexEnterCar 系列（7 种变体）— 驾驶员/乘客/定时/等待
- TaskComplexLeaveCar 系列（6 种变体）— 离开/逃跑/徘徊/死亡
- TaskComplexEnterBoatAsDriver, TaskComplexLeaveBoat
- TaskComplexDragPedFromCar, TaskComplexCarSlowBeDraggedOut

**车辆动画（Simple 级）:**
- TaskSimpleCarDrive, TaskSimpleCarGetIn/Out, TaskSimpleCarAlign
- TaskSimpleCarOpenDoorFromOutside, TaskSimpleCarCloseDoorFromInside/Outside
- TaskSimpleCarSetPedInAsDriver/Passenger, TaskSimpleCarSetPedOut
- TaskSimpleBikeJacked, TaskSimplePickUpBike

**特殊任务:**
- TaskComplexStealCar, TaskComplexDestroyCar（3 种变体）
- TaskComplexHitPedWithCar, TaskComplexKillPedFromBoat
- TaskComplexScreamInCarThenLeave, TaskComplexWalkRoundCar
- TaskGangHassleVehicle, TaskGoToVehicleAndLean

### 载具相关事件 — Events/（17 个事件类）
- EventVehicleCollision, EventVehicleDamage, EventVehicleDamageCollision, EventVehicleDamageWeapon
- EventVehicleDied, EventVehicleOnFire, EventVehicleThreat, EventVehicleToSteal, EventVehicleHitAndRun
- EventSexyVehicle, EventPedEnteredMyVehicle, EventLeanOnVehicle
- EventCarUpsideDown, EventPotentialWalkIntoVehicle
- EventDraggedOutCar, EventKnockOffBike, EventGotKnockedOverByCar
- EventCopCarBeingStolen, EventLeaderEnteredCarAsDriver
- VehiclePotentialCollisionScanner — 碰撞威胁扫描

### 枚举定义 — Enums/
- eVehicleType.h — 载具类型
- eVehicleClass.h — 载具分类（Normal, Lowrider, RC 等）
- eVehicleHandlingFlags.h — 操控标志
- eVehicleHandlingModelFlags.h — 模型操控标志
- eCarMission.h — AI 驾驶任务（巡航、追逐、拦截、撞击、护送等）
- eCarDrivingStyle.h — 驾驶风格
- eCarNodes.h — 汽车 DFF 节点索引

### 脚本命令
- Scripts/Commands/VehicleCommands.cpp — SCM 脚本载具命令
- Scripts/Commands/CLEO/CLEOVehicleCommands.cpp — CLEO 扩展载具命令

## 6. 代码量统计

| 文件 | 行数 |
|------|------|
| Automobile.cpp | 6,511 |
| Vehicle.cpp | 4,594 |
| Boat.cpp | 1,108 |
| Train.cpp | 925 |
| Vehicle.h | 804 |
| Bike.cpp | 631 |
| Trailer.cpp | 566 |
| Plane.cpp | 450 |
| Automobile.h | 417 |
| Heli.cpp | 352 |
| QuadBike.cpp | 313 |
| Bmx.cpp | 220 |
| MonsterTruck.cpp | 182 |
| Bike.h | 165 |
| **全部载具文件合计** | **18,110** |

## 7. 总结

载具系统是 gta-reversed 项目中规模最大的子系统之一，涉及 731+ 个函数、89 个任务类、17 个事件类。

**优势:**
- 基础架构扎实: CVehicle + CAutomobile 合计 234 个函数，97-98% 完成
- 管理器层几乎完工: CCarAI, CVehicleRecording, cHandlingDataMgr, CCarGenerator, CVehicleModelInfo 均 100%
- CBoat, CHeli, CTrailer 已 100% 逆向

**主要缺口:**
- CTrain (50%) — 20 个函数，轨道系统管理和路径数学
- CBike (52.5%) — 19 个函数，核心 ProcessControl/VehicleDamage 等
- CPlane (60.9%) — 9 个函数，飞行模拟核心循环
- CMonsterTruck (45.5%) — 6 个函数，悬挂和物理控制管线
- Vehicle.cpp 中 26 个已定位但未开始逆向的注释掉的 hook

**整体完成率: 84.9%（核心载具类 359/423）**
