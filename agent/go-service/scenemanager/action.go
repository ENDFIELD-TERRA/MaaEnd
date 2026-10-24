package scenemanager

import (
	maa "github.com/MaaXYZ/maa-framework-go/v4"

	"github.com/MaaXYZ/MaaEnd/agent/go-service/pkg/pienv"
)

type SceneManagerMenuListClickItemAction struct{}

// Compile-time interface check
var _ maa.CustomActionRunner = &SceneManagerMenuListClickItemAction{}

func (a *SceneManagerMenuListClickItemAction) Run(ctx *maa.Context, arg *maa.CustomActionArg) bool {
	// https://github.com/MaaEnd/MaaEnd/issues/1456

	// 取 Box 上边中点并向上偏移 15px
	x := arg.Box.X() + arg.Box.Width()/2
	y := arg.Box.Y() - 15
	if pienv.ResolveControllerKind(nil) == pienv.ControllerKindCloudWeb {
		// CloudWeb 的 3D 菜单命中框与截图标签不是同一坐标：实测
		// "Regional Development" 标签中心 (1138,247) 的命中点是
		// (1100,240)。用 Box 中心减去该偏移；桌面端保持原偏移。
		x = arg.Box.X() + arg.Box.Width()/2 - 38
		y = arg.Box.Y() + arg.Box.Height()/2 - 7
	}
	target := maa.Rect{x, y, 1, 1}
	ctx.RunAction("__SceneClickAction",
		target, "", nil)
	return true
}
