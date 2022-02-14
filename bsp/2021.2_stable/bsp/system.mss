
 PARAMETER VERSION = 2.2.0


BEGIN OS
 PARAMETER OS_NAME = freertos10_xilinx
 PARAMETER OS_VER = 1.10
 PARAMETER PROC_INSTANCE = blp_cips_pspmc_0_psv_cortexr5_0
 PARAMETER SYSINTC_SPEC = *
 PARAMETER SYSTMR_DEV = *
 PARAMETER SYSTMR_SPEC = true
 PARAMETER stdin = blp_cips_pspmc_0_psv_sbsauart_1
 PARAMETER stdout = blp_cips_pspmc_0_psv_sbsauart_1
END


BEGIN PROCESSOR
 PARAMETER DRIVER_NAME = cpu_cortexr5
 PARAMETER DRIVER_VER = 2.0
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_cortexr5_0
END


BEGIN DRIVER
 PARAMETER DRIVER_NAME = ddrpsv
 PARAMETER DRIVER_VER = 1.4
 PARAMETER HW_INSTANCE = blp_axi_noc_mc
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_blp_logic_axi_blp_dbg_hub
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_blp_logic_axi_firewall_user
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartlite
 PARAMETER DRIVER_VER = 3.6
 PARAMETER HW_INSTANCE = blp_blp_logic_axi_uart_apu
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartlite
 PARAMETER DRIVER_VER = 3.6
 PARAMETER HW_INSTANCE = blp_blp_logic_axi_uart_mgmt_apu
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartlite
 PARAMETER DRIVER_VER = 3.6
 PARAMETER HW_INSTANCE = blp_blp_logic_axi_uart_mgmt_rpu
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartlite
 PARAMETER DRIVER_VER = 3.6
 PARAMETER HW_INSTANCE = blp_blp_logic_axi_uart_rpu
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = gpio
 PARAMETER DRIVER_VER = 4.8
 PARAMETER HW_INSTANCE = blp_blp_logic_base_clocking_force_reset_gpio
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = gpio
 PARAMETER DRIVER_VER = 4.8
 PARAMETER HW_INSTANCE = blp_blp_logic_base_clocking_pr_reset_gpio
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = intc
 PARAMETER DRIVER_VER = 3.13
 PARAMETER HW_INSTANCE = blp_blp_logic_ert_support_axi_intc_0_31
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_blp_logic_pf0_bar_layout
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_blp_logic_pf1_bar_layout
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = mbox
 PARAMETER DRIVER_VER = 4.5
 PARAMETER HW_INSTANCE = blp_blp_logic_pf_mailbox
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_blp_logic_sb_remap
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = clk_wiz
 PARAMETER DRIVER_VER = 1.4
 PARAMETER HW_INSTANCE = blp_blp_logic_ulp_clocking_aclk_kernel_00_hierarchy_clkwiz_aclk_kernel_00
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = clk_wiz
 PARAMETER DRIVER_VER = 1.4
 PARAMETER HW_INSTANCE = blp_blp_logic_ulp_clocking_aclk_kernel_01_hierarchy_clkwiz_aclk_kernel_01
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_blp_logic_ulp_clocking_frequency_counters_frequency_counter_aclk
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_blp_logic_ulp_clocking_frequency_counters_frequency_counter_aclk_kernel_00
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_blp_logic_ulp_clocking_frequency_counters_frequency_counter_aclk_kernel_01
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = gpio
 PARAMETER DRIVER_VER = 4.8
 PARAMETER HW_INSTANCE = blp_blp_logic_ulp_clocking_gapping_demand_gpio_gapping_demand
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = gpio
 PARAMETER DRIVER_VER = 4.8
 PARAMETER HW_INSTANCE = blp_blp_logic_ulp_clocking_ucs_control_status_gpio_ucs_control_status
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_blp_logic_uuid_rom
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_blp_logic_xgq_m2r
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_blp_logic_xgq_r2a
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_blp_logic_xgq_u2a_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_blp_logic_xgq_u2a_1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_blp_logic_xgq_u2a_2
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_blp_logic_xgq_u2a_3
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = scugic
 PARAMETER DRIVER_VER = 4.6
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_acpu_gic
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = zdma
 PARAMETER DRIVER_VER = 1.13
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_adma_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = zdma
 PARAMETER DRIVER_VER = 1.13
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_adma_1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = zdma
 PARAMETER DRIVER_VER = 1.13
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_adma_2
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = zdma
 PARAMETER DRIVER_VER = 1.13
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_adma_3
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = zdma
 PARAMETER DRIVER_VER = 1.13
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_adma_4
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = zdma
 PARAMETER DRIVER_VER = 1.13
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_adma_5
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = zdma
 PARAMETER DRIVER_VER = 1.13
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_adma_6
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = zdma
 PARAMETER DRIVER_VER = 1.13
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_adma_7
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_apu_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = coresightps_dcc
 PARAMETER DRIVER_VER = 1.8
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_coresight_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = pmonpsv
 PARAMETER DRIVER_VER = 2.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_coresight_fpd_atm
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_coresight_fpd_stm
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = pmonpsv
 PARAMETER DRIVER_VER = 2.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_coresight_lpd_atm
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_crf_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_crl_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_crp_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_fpd_afi_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_fpd_afi_2
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_fpd_cci_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_fpd_gpv_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_fpd_maincci_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_fpd_slave_xmpu_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_fpd_slcr_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_fpd_slcr_secure_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_fpd_smmu_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_fpd_smmutcu_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = gpiops
 PARAMETER DRIVER_VER = 3.9
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_gpio_2
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = iicps
 PARAMETER DRIVER_VER = 3.14
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_i2c_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = iicps
 PARAMETER DRIVER_VER = 3.14
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_i2c_1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ipipsu
 PARAMETER DRIVER_VER = 2.10
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_ipi_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ipipsu
 PARAMETER DRIVER_VER = 2.10
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_ipi_1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ipipsu
 PARAMETER DRIVER_VER = 2.10
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_ipi_2
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ipipsu
 PARAMETER DRIVER_VER = 2.10
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_ipi_3
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ipipsu
 PARAMETER DRIVER_VER = 2.10
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_ipi_4
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ipipsu
 PARAMETER DRIVER_VER = 2.10
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_ipi_5
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ipipsu
 PARAMETER DRIVER_VER = 2.10
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_ipi_6
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ipipsu
 PARAMETER DRIVER_VER = 2.10
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_ipi_pmc
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ipipsu
 PARAMETER DRIVER_VER = 2.10
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_ipi_pmc_nobuf
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ipipsu
 PARAMETER DRIVER_VER = 2.10
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_ipi_psm
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_lpd_afi_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_lpd_iou_secure_slcr_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_lpd_iou_slcr_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_lpd_slcr_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_lpd_slcr_secure_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_lpd_xppu_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_ocm_ctrl
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_ocm_ram_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_ocm_xmpu_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_aes
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_bbram_ctrl
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = cframe
 PARAMETER DRIVER_VER = 1.3
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_cfi_cframe_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = cfupmc
 PARAMETER DRIVER_VER = 1.3
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_cfu_apb_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = csudma
 PARAMETER DRIVER_VER = 1.10
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_dma_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = csudma
 PARAMETER DRIVER_VER = 1.10
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_dma_1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_efuse_cache
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_efuse_ctrl
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_global_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = gpiops
 PARAMETER DRIVER_VER = 3.9
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_gpio_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = iicps
 PARAMETER DRIVER_VER = 3.14
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_i2c_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ospipsv
 PARAMETER DRIVER_VER = 1.5
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_ospi_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartlite
 PARAMETER DRIVER_VER = 3.6
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_ppu1_mdm_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_qspi_ospi_flash_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_ram
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_ram_npi
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_rsa
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = rtcpsu
 PARAMETER DRIVER_VER = 1.11
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_rtc_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_sha
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = sysmonpsv
 PARAMETER DRIVER_VER = 2.3
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_sysmon_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = trngpsv
 PARAMETER DRIVER_VER = 1.0
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_trng
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_xmpu_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_xppu_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_pmc_xppu_npi_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_psm_global_reg
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_r5_0_atcm
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_r5_0_btcm
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_r5_0_data_cache
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_r5_0_instruction_cache
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_r5_1_data_cache
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_r5_1_instruction_cache
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_r5_tcm_ram_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = scugic
 PARAMETER DRIVER_VER = 4.6
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_rcpu_gic
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_rpu_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartpsv
 PARAMETER DRIVER_VER = 1.5
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_sbsauart_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartpsv
 PARAMETER DRIVER_VER = 1.5
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_sbsauart_1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_scntr_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_scntrs_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ttcps
 PARAMETER DRIVER_VER = 3.14
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_ttc_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ttcps
 PARAMETER DRIVER_VER = 3.14
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_ttc_1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ttcps
 PARAMETER DRIVER_VER = 3.14
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_ttc_2
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ttcps
 PARAMETER DRIVER_VER = 3.14
 PARAMETER HW_INSTANCE = blp_cips_pspmc_0_psv_ttc_3
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_shell_utils_remap_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = blp_shell_utils_remap_1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 3.1
 PARAMETER HW_INSTANCE = ulp_axi_dbg_hub
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = gpio
 PARAMETER DRIVER_VER = 4.8
 PARAMETER HW_INSTANCE = ulp_axi_gpio_null_user
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = bram
 PARAMETER DRIVER_VER = 4.7
 PARAMETER HW_INSTANCE = ulp_plram_ctrl
END


BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilfpga
 PARAMETER LIBRARY_VER = 6.1
 PARAMETER PROC_INSTANCE = blp_cips_pspmc_0_psv_cortexr5_0
END


BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilmailbox
 PARAMETER LIBRARY_VER = 1.4
 PARAMETER PROC_INSTANCE = blp_cips_pspmc_0_psv_cortexr5_0
END


