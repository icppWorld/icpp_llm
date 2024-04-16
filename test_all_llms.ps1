#######################################################################
# This is a Windows PowerShell script, 
#
# (-) equivalent to the Makefile target `test-all-llms:
# (-) In a Windows PowerShell (*):
#
#     .\test_all_llms.ps1
#
# (*) The Miniconda Powershell is highly recommended
#
#######################################################################

wsl --% dfx identity use default

Write-Host " "
Write-Host "--------------------------------------------------"
Write-Host "Testing llama2_c"
Set-Location -Path .\llama2_c
.\demo.ps1

# Change directory back to the root
Set-Location -Path ..\..