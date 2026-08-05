addhandler leopard_dpu_deprecated_warning
python leopard_dpu_deprecated_warning() {
    if e.data.getVar('MACHINE') == 'leopard-dpu':
        bb.warn("Deprecated: leopard-dpu machine is deprecated and will be removed in one of future releases, use leopard-dpu-<zu9eg|zu15eg> instead.")
}
leopard_dpu_deprecated_warning[eventmask] = "bb.event.BuildStarted"
