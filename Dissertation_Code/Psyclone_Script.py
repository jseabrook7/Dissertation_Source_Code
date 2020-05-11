def trans(psy):
    schedule = psy.invokes.invoke_list[0].schedule
    loop = schedule[0]
    kernel = loop.loop_body[0]
    kernel_schedule = kernel.get_kernel_schedule()
    assignment = kernel_schedule[0]
    rhs = assignment.rhs

    loop_start = loop.start_expr.value
    loop_end = loop.stop_expr.value
    loop_step = loop.step_expr.value
    assignment_value = rhs.value

    f = open("Variables.txt", "w+")
    f.write(loop_start + "\n")
    f.write(loop_end + "\n")
    f.write(loop_step + "\n")
    f.write(assignment_value)
    f.close()
    exit(1)
