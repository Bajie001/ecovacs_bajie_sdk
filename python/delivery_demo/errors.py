from typing import Optional


class DeliveryDemoError(Exception):
    """Base error for delivery workflow."""


class ValidationError(DeliveryDemoError):
    """Raised when workflow inputs or intermediate data are invalid."""


class RobotMissionError(DeliveryDemoError):
    """Raised when a robot mission fails."""

    def __init__(self, message: str, *, code: Optional[int] = None, msg: Optional[str] = None):
        super().__init__(message)
        self.code = code
        self.msg = msg

    @classmethod
    def from_error_info(cls, step: str, error_info) -> "RobotMissionError":
        return cls(
            f"{step} failed: code={error_info.code}, msg={error_info.msg}",
            code=error_info.code,
            msg=error_info.msg,
        )


class HttpServiceError(DeliveryDemoError):
    """Raised when HTTP perception service fails."""
